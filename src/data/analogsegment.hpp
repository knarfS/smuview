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

#ifndef DATA_ANALOGSEGMENT_HPP
#define DATA_ANALOGSEGMENT_HPP

#include "src/util.hpp"

#include <atomic>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include <QObject>

using std::atomic;
using std::deque;
using std::enable_shared_from_this;
using std::pair;
using std::recursive_mutex;

/*
namespace SegmentTest {
struct SmallSize8Single;
struct MediumSize8Single;
struct MaxSize8Single;
struct MediumSize24Single;
struct MediumSize32Single;
struct MaxSize32Single;
struct MediumSize32Multi;
struct MaxSize32Multi;
struct MaxSize32MultiAtOnce;
struct MaxSize32MultiIterated;
}  // namespace SegmentTest
*/

/*
namespace AnalogSegmentTest {
struct Basic;
}
*/


//typedef std::shared_ptr<sv::data::AnalogSegment> SharedPtrToSegment;
//Q_DECLARE_METATYPE(std::shared_ptr<sv::data::AnalogSegment>);

namespace sv {
namespace data {

class AnalogScopeSignal;

typedef struct {
	uint64_t sample_index, chunk_num, chunk_offs;
	uint8_t* chunk;
} SegmentDataIterator;

class AnalogSegment : public QObject, public enable_shared_from_this<AnalogSegment>
{
	Q_OBJECT

public:
	struct EnvelopeSample
	{
		float min;
		float max;
	};

	struct EnvelopeSection
	{
		uint64_t start;
		unsigned int scale;
		uint64_t length;
		EnvelopeSample *samples;
	};

private:
	struct Envelope
	{
		uint64_t length;
		uint64_t data_length;
		EnvelopeSample *samples;
	};

private:
	static const uint64_t MaxChunkSize;

	static const unsigned int ScaleStepCount = 10;
	static const int EnvelopeScalePower;
	static const int EnvelopeScaleFactor;
	static const float LogEnvelopeScaleFactor;
	static const uint64_t EnvelopeDataUnit;

public:
	AnalogSegment(AnalogScopeSignal& owner,
		uint32_t segment_id, uint64_t samplerate);
	virtual ~AnalogSegment();

	uint32_t id() const;

	uint64_t sample_count() const;

	const sv::util::Timestamp& start_time() const;

	void set_samplerate(uint64_t samplerate);
	uint64_t samplerate() const;
	double time_stride() const;

	unsigned int unit_size() const;

	void set_complete();
	bool is_complete() const;

	void free_unused_memory();

	// ---

	void append_interleaved_samples(const float *data,
		size_t sample_count, size_t stride);

	float get_sample(int64_t sample_num) const;
	void get_samples(int64_t start_sample, int64_t end_sample, float* dest) const;

	const pair<float, float> get_min_max() const;

	float* get_iterator_value_ptr(SegmentDataIterator* it);

	void get_envelope_section(EnvelopeSection &s,
		uint64_t start, uint64_t end, float min_length) const;

private:
	void append_single_sample(void *data);
	void append_samples(void *data, uint64_t samples);
	const uint8_t* get_raw_sample(uint64_t sample_num) const;
	void get_raw_samples(uint64_t start, uint64_t count, uint8_t *dest) const;

	SegmentDataIterator* begin_sample_iteration(uint64_t start);
	void continue_sample_iteration(SegmentDataIterator* it, uint64_t increase);
	void end_sample_iteration(SegmentDataIterator* it);
	uint8_t* get_iterator_value(SegmentDataIterator* it);
	uint64_t get_iterator_valid_length(SegmentDataIterator* it);

	// ---

	void reallocate_envelope(Envelope &e);

	void append_payload_to_envelope_levels();

private:
	uint32_t id_;
	mutable recursive_mutex mutex_;
	deque<uint8_t*> data_chunks_;
	uint8_t* current_chunk_;
	uint64_t used_samples_, unused_samples_;
	atomic<uint64_t> sample_count_;
	sv::util::Timestamp start_time_;
	uint64_t samplerate_;
	double time_stride_;
	uint64_t chunk_size_;
	unsigned int unit_size_;
	int iterator_count_;
	bool mem_optimization_requested_;
	bool is_complete_;

	// ---

	AnalogScopeSignal &owner_;

	struct Envelope envelope_levels_[ScaleStepCount];

	float min_value_, max_value_;

Q_SIGNALS:
	void samples_added(uint32_t segment_id);
	void completed(uint32_t segment_id);

};

} // namespace data
} // namespace sv

#endif // DATA_ANALOGSEGMENT_HPP
