/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <extdef.h>

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>

#include <algorithm>

#include <QDebug>

#include "analogsegment.hpp"
#include "src/data/analogscopesignal.hpp"

using std::bad_alloc;
using std::lock_guard;
using std::recursive_mutex;
using std::make_pair;
using std::max;
using std::max_element;
using std::min;
using std::min_element;
using std::pair;
using std::unique_ptr;

namespace sv {
namespace data {

const uint64_t AnalogSegment::MaxChunkSize = 10 * 1024 * 1024;  /* 10MiB */

const int AnalogSegment::EnvelopeScalePower = 4;
const int AnalogSegment::EnvelopeScaleFactor = 1 << EnvelopeScalePower;
const float AnalogSegment::LogEnvelopeScaleFactor = logf(EnvelopeScaleFactor);
const uint64_t AnalogSegment::EnvelopeDataUnit = 64 * 1024;	// bytes

AnalogSegment::AnalogSegment(AnalogScopeSignal& owner,
		uint32_t segment_id, uint64_t samplerate) :
	id_(segment_id),
	sample_count_(0),
	start_time_(0),
	unit_size_(sizeof(float)),
	iterator_count_(0),
	mem_optimization_requested_(false),
	is_complete_(false),
	owner_(owner),
	min_value_(.0),
	max_value_(.0)
{
	assert(unit_size_ > 0);

	set_samplerate(samplerate);

	// Determine the number of samples we can fit in one chunk
	// without exceeding MaxChunkSize
	chunk_size_ = min(MaxChunkSize, (MaxChunkSize / unit_size_) * unit_size_);

	// Create the initial chunk
	current_chunk_ = new uint8_t[chunk_size_ + 7];  /* FIXME +7 is workaround for #1284 */
	data_chunks_.push_back(current_chunk_);
	used_samples_ = 0;
	unused_samples_ = chunk_size_ / unit_size_;

	lock_guard<recursive_mutex> lock(mutex_);
	memset(envelope_levels_, 0, sizeof(envelope_levels_));
}

AnalogSegment::~AnalogSegment()
{
	lock_guard<recursive_mutex> lock(mutex_);

	for (Envelope &e : envelope_levels_)
		free(e.samples);

	for (uint8_t* chunk : data_chunks_)
		delete[] chunk;
}

void AnalogSegment::append_interleaved_samples(const float *data,
	size_t sample_count, size_t stride)
{
	assert(unit_size_ == sizeof(float));

	lock_guard<recursive_mutex> lock(mutex_);

	//uint64_t prev_sample_count = sample_count_;

	// Deinterleave the samples and add them
	unique_ptr<float[]> deint_data(new float[sample_count]);
	float *deint_data_ptr = deint_data.get();
	for (uint32_t i = 0; i < sample_count; i++) {
		*deint_data_ptr = (float)(*data);
		deint_data_ptr++;
		data += stride;
	}

	append_samples(deint_data.get(), sample_count);

	// Generate the first mip-map from the data
	append_payload_to_envelope_levels();

	Q_EMIT samples_added(id_);
	/*
	if (sample_count > 1) {
		owner_.notify_samples_added(shared_ptr<Segment>(shared_from_this()),
			prev_sample_count + 1, prev_sample_count + 1 + sample_count);
	}
	else {
		owner_.notify_samples_added(shared_ptr<Segment>(shared_from_this()),
			prev_sample_count + 1, prev_sample_count + 1);
	}
	*/
}

float AnalogSegment::get_sample(int64_t sample_num) const
{
	assert(sample_num >= 0);
	assert(sample_num <= (int64_t)sample_count_);

	lock_guard<recursive_mutex> lock(mutex_);  // Because of free_unused_memory()

	return *((const float*)get_raw_sample(sample_num));
}

void AnalogSegment::get_samples(int64_t start_sample, int64_t end_sample,
	float* dest) const
{
	assert(start_sample >= 0);
	assert(start_sample < (int64_t)sample_count_);
	assert(end_sample >= 0);
	assert(end_sample <= (int64_t)sample_count_);
	assert(start_sample <= end_sample);
	assert(dest != nullptr);

	lock_guard<recursive_mutex> lock(mutex_);

	get_raw_samples(start_sample, (end_sample - start_sample), (uint8_t*)dest);
}

float AnalogSegment::min_value() const
{
	return min_value_;
}

float AnalogSegment::max_value() const
{
	return max_value_;
}

float* AnalogSegment::get_iterator_value_ptr(SegmentDataIterator* it)
{
	assert(it->sample_index <= (sample_count_ - 1));

	return (float*)(it->chunk + it->chunk_offs);
}

void AnalogSegment::get_envelope_section(EnvelopeSection &s,
	uint64_t start, uint64_t end, float min_length) const
{
	assert(end <= sample_count_);
	assert(start <= end);
	assert(min_length > 0);

	lock_guard<recursive_mutex> lock(mutex_);

	const unsigned int min_level = max((int)floorf(logf(min_length) /
		LogEnvelopeScaleFactor) - 1, 0);
	const unsigned int scale_power = (min_level + 1) *
		EnvelopeScalePower;
	start >>= scale_power;
	end >>= scale_power;

	s.start = start; // << scale_power; // BUG
	s.scale = 1 << scale_power;
	s.length = end - start;
	s.time_stride = time_stride_ * s.scale; //envelope_levels_[min_level].time_stride;
	s.samples = new EnvelopeSample[s.length];
	memcpy(s.samples, envelope_levels_[min_level].samples + start,
		s.length * sizeof(EnvelopeSample));
}

void AnalogSegment::reallocate_envelope(Envelope &e)
{
	const uint64_t new_data_length = ((e.length + EnvelopeDataUnit - 1) /
		EnvelopeDataUnit) * EnvelopeDataUnit;
	if (new_data_length > e.data_length) {
		e.data_length = new_data_length;
		e.samples = (EnvelopeSample*)realloc(e.samples,
			new_data_length * sizeof(EnvelopeSample));
	}
}

void AnalogSegment::append_payload_to_envelope_levels()
{
	Envelope &e0 = envelope_levels_[0];
	uint64_t prev_length;
	EnvelopeSample *dest_ptr;
	SegmentDataIterator* it;

	// Expand the data buffer to fit the new samples
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): e0.length = " << e0.length;
	prev_length = e0.length;
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): sample_count_ / EnvelopeScaleFactor = "
	//	<< sample_count_ << "/" << EnvelopeScaleFactor;
	e0.length = sample_count_ / EnvelopeScaleFactor;
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): e0.length = " << e0.length;
	e0.time_stride = time_stride_ * EnvelopeScaleFactor;

	// Calculate min/max values in case we have too few samples for an envelope
	const float old_min_value = min_value_;
	const float old_max_value = max_value_;
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): old_min_value = " << old_min_value;
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): old_max_value = " << old_max_value;
	// TODO: Use better min/max algorithm with faster comparisons of only 3N/2 instead of 2N
	if (sample_count_ < EnvelopeScaleFactor) {
		it = begin_sample_iteration(0);
		for (uint64_t i = 0; i < sample_count_; i++) {
			const float sample = *get_iterator_value_ptr(it);
			if (sample < min_value_)
				min_value_ = sample;
			if (sample > max_value_)
				max_value_ = sample;
			continue_sample_iteration(it, 1);
		}
		end_sample_iteration(it);
	}
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): envelope_min_value_ = " << envelope_min_value_;
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): envelope_max_value_ = " << envelope_max_value_;

	// Break off if there are no new samples to compute
	if (e0.length == prev_length)
		return;

	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): reallocate_envelope";
	reallocate_envelope(e0);

	dest_ptr = e0.samples + prev_length;

	// Iterate through the samples to populate the first level mipmap
	uint64_t start_sample = prev_length * EnvelopeScaleFactor;
	uint64_t end_sample = e0.length * EnvelopeScaleFactor;
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): start_sample = " << start_sample;
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): end_sample = " << end_sample;

	it = begin_sample_iteration(start_sample);
	for (uint64_t i = start_sample; i < end_sample; i += EnvelopeScaleFactor) {
		//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): i = " << i;
		const float* samples = get_iterator_value_ptr(it);

		const EnvelopeSample sub_sample = {
			*min_element(samples, samples + EnvelopeScaleFactor),
			*max_element(samples, samples + EnvelopeScaleFactor),
		};

		// TODO: Use better min/max algorithm with faster comparisons of only 3N/2 instead of 2N
		if (sub_sample.min < min_value_)
			min_value_ = sub_sample.min;
		if (sub_sample.max > max_value_)
			max_value_ = sub_sample.max;

		continue_sample_iteration(it, EnvelopeScaleFactor);
		*dest_ptr++ = sub_sample;
	}
	end_sample_iteration(it);
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): envelope_min_value_ = " << envelope_min_value_;
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): envelope_max_value_ = " << envelope_max_value_;

	// Compute higher level mipmaps
	for (unsigned int level = 1; level < ScaleStepCount; level++) {
		//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): level = " << level;
		Envelope &e = envelope_levels_[level];
		const Envelope &el = envelope_levels_[level - 1];

		// Expand the data buffer to fit the new samples
		prev_length = e.length;
		//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): e.length = " << e.length;
		e.length = el.length / EnvelopeScaleFactor;
		//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): e.length = " << e.length;
		e.time_stride = time_stride_ * EnvelopeScaleFactor * level;

		// Break off if there are no more samples to be computed
		if (e.length == prev_length)
			break;

		//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): reallocate_envelope";
		reallocate_envelope(e);

		// Subsample the lower level
		const EnvelopeSample *src_ptr =
			el.samples + prev_length * EnvelopeScaleFactor;
		const EnvelopeSample *const end_dest_ptr = e.samples + e.length;

		for (dest_ptr = e.samples + prev_length;
				dest_ptr < end_dest_ptr; dest_ptr++) {
			const EnvelopeSample *const end_src_ptr =
				src_ptr + EnvelopeScaleFactor;

			EnvelopeSample sub_sample = *src_ptr++;
			while (src_ptr < end_src_ptr) {
				sub_sample.min = min(sub_sample.min, src_ptr->min);;
				sub_sample.max = max(sub_sample.max, src_ptr->max);
				src_ptr++;
			}
			//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): sub_sample.min = " << sub_sample.min;
			//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): sub_sample.max = " << sub_sample.max;

			*dest_ptr = sub_sample;
		}
	}

	// Notify if the min or max value changed
	if ((old_min_value != min_value_) || (old_max_value != max_value_)) {
		// TODO
		//owner_.min_max_changed(min_value_, max_value_);
	}
	//qWarning() << "AnalogSegment::append_payload_to_envelope_levels(): donesky";
}

// ----

uint64_t AnalogSegment::sample_count() const
{
	return sample_count_;
}

const sv::util::Timestamp& AnalogSegment::start_time() const
{
	return start_time_;
}

void AnalogSegment::set_samplerate(uint64_t samplerate)
{
	samplerate_ = samplerate;
	time_stride_ = 1. / (double)samplerate_;
}

uint64_t AnalogSegment::samplerate() const
{
	return samplerate_;
}

double AnalogSegment::time_stride() const
{
	return time_stride_;
}

unsigned int AnalogSegment::unit_size() const
{
	return unit_size_;
}

uint32_t AnalogSegment::id() const
{
	return id_;
}

void AnalogSegment::set_complete()
{
	is_complete_ = true;
	Q_EMIT completed(id_);
}

bool AnalogSegment::is_complete() const
{
	return is_complete_;
}

void AnalogSegment::free_unused_memory()
{
	lock_guard<recursive_mutex> lock(mutex_);

	// Do not mess with the data chunks if we have iterators pointing at them
	if (iterator_count_ > 0) {
		mem_optimization_requested_ = true;
		return;
	}

	if (current_chunk_) {
		// No more data will come in, so re-create the last chunk accordingly
		uint8_t* resized_chunk = new uint8_t[used_samples_ * unit_size_ + 7];  /* FIXME +7 is workaround for #1284 */
		memcpy(resized_chunk, current_chunk_, used_samples_ * unit_size_);

		delete[] current_chunk_;
		current_chunk_ = resized_chunk;

		data_chunks_.pop_back();
		data_chunks_.push_back(resized_chunk);
	}
}

void AnalogSegment::append_single_sample(void *data)
{
	lock_guard<recursive_mutex> lock(mutex_);

	// There will always be space for at least one sample in
	// the current chunk, so we do not need to test for space

	memcpy(current_chunk_ + (used_samples_ * unit_size_), data, unit_size_);
	used_samples_++;
	unused_samples_--;

	if (unused_samples_ == 0) {
		current_chunk_ = new uint8_t[chunk_size_ + 7];  /* FIXME +7 is workaround for #1284 */
		data_chunks_.push_back(current_chunk_);
		used_samples_ = 0;
		unused_samples_ = chunk_size_ / unit_size_;
	}

	sample_count_++;
}

void AnalogSegment::append_samples(void* data, uint64_t samples)
{
	lock_guard<recursive_mutex> lock(mutex_);

	const uint8_t* data_byte_ptr = (uint8_t*)data;
	uint64_t remaining_samples = samples;
	uint64_t data_offset = 0;

	do {
		uint64_t copy_count = 0;

		if (remaining_samples <= unused_samples_) {
			// All samples fit into the current chunk
			copy_count = remaining_samples;
		}
		else {
			// Only a part of the samples fit, fill up current chunk
			copy_count = unused_samples_;
		}

		const uint8_t* dest = &(current_chunk_[used_samples_ * unit_size_]);
		const uint8_t* src = &(data_byte_ptr[data_offset]);
		memcpy((void*)dest, (void*)src, (copy_count * unit_size_));

		used_samples_ += copy_count;
		unused_samples_ -= copy_count;
		remaining_samples -= copy_count;
		data_offset += (copy_count * unit_size_);

		if (unused_samples_ == 0) {
			try {
				// If we're out of memory, allocating a chunk will throw
				// std::bad_alloc. To give the application some usable memory
				// to work with in case chunk allocation fails, we allocate
				// extra memory and throw it away if it all succeeded.
				// This way, memory allocation will fail early enough to let
				// PV remain alive. Otherwise, PV will crash in a random
				// memory-allocating part of the application.
				current_chunk_ = new uint8_t[chunk_size_ + 7];  /* FIXME +7 is workaround for #1284 */

				const int dummy_size = 2 * chunk_size_;
				auto dummy_chunk = new uint8_t[dummy_size];
				memset(dummy_chunk, 0xFF, dummy_size);
				delete[] dummy_chunk;
			}
			catch (bad_alloc&) {
				delete[] current_chunk_;  // The new may have succeeded
				current_chunk_ = nullptr;
				throw;
			}

			data_chunks_.push_back(current_chunk_);
			used_samples_ = 0;
			unused_samples_ = chunk_size_ / unit_size_;
		}
	}
	while (remaining_samples > 0);

	sample_count_ += samples;
}

const uint8_t* AnalogSegment::get_raw_sample(uint64_t sample_num) const
{
	assert(sample_num <= sample_count_);

	uint64_t chunk_num = (sample_num * unit_size_) / chunk_size_;
	uint64_t chunk_offs = (sample_num * unit_size_) % chunk_size_;

	lock_guard<recursive_mutex> lock(mutex_);  // Because of free_unused_memory()

	const uint8_t* chunk = data_chunks_[chunk_num];

	return chunk + chunk_offs;
}

void AnalogSegment::get_raw_samples(uint64_t start, uint64_t count, uint8_t* dest) const
{
	assert(start < sample_count_);
	assert(start + count <= sample_count_);
	assert(count > 0);
	assert(dest != nullptr);

	uint8_t* dest_ptr = dest;

	uint64_t chunk_num = (start * unit_size_) / chunk_size_;
	uint64_t chunk_offs = (start * unit_size_) % chunk_size_;

	lock_guard<recursive_mutex> lock(mutex_);  // Because of free_unused_memory()

	while (count > 0) {
		const uint8_t* chunk = data_chunks_[chunk_num];

		uint64_t copy_size = min(count * unit_size_,
			chunk_size_ - chunk_offs);

		memcpy(dest_ptr, chunk + chunk_offs, copy_size);

		dest_ptr += copy_size;
		count -= (copy_size / unit_size_);

		chunk_num++;
		chunk_offs = 0;
	}
}

SegmentDataIterator* AnalogSegment::begin_sample_iteration(uint64_t start)
{
	SegmentDataIterator* it = new SegmentDataIterator;

	assert(start < sample_count_);

	iterator_count_++;

	it->sample_index = start;
	it->chunk_num = (start * unit_size_) / chunk_size_;
	it->chunk_offs = (start * unit_size_) % chunk_size_;
	it->chunk = data_chunks_[it->chunk_num];

	return it;
}

void AnalogSegment::continue_sample_iteration(SegmentDataIterator* it, uint64_t increase)
{
	it->sample_index += increase;
	it->chunk_offs += (increase * unit_size_);

	if (it->chunk_offs > (chunk_size_ - 1)) {
		it->chunk_num++;
		it->chunk_offs -= chunk_size_;
		it->chunk = data_chunks_[it->chunk_num];
	}
}

void AnalogSegment::end_sample_iteration(SegmentDataIterator* it)
{
	delete it;

	iterator_count_--;

	if ((iterator_count_ == 0) && mem_optimization_requested_) {
		mem_optimization_requested_ = false;
		free_unused_memory();
	}
}

uint8_t* AnalogSegment::get_iterator_value(SegmentDataIterator* it)
{
	assert(it->sample_index <= (sample_count_ - 1));

	return (it->chunk + it->chunk_offs);
}

uint64_t AnalogSegment::get_iterator_valid_length(SegmentDataIterator* it)
{
	assert(it->sample_index <= (sample_count_ - 1));

	return ((chunk_size_ - it->chunk_offs) / unit_size_);
}

} // namespace data
} // namespace sv
