/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <algorithm>
#include <cassert>
#include <memory>
#include <set>

#include <QDebug>
#include <QString>

#include "analogscopesignal.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsegment.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"

using std::make_pair;
using std::make_shared;
using std::set;
using std::shared_ptr;
using std::vector;

namespace sv {
namespace data {

AnalogScopeSignal::AnalogScopeSignal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		double signal_start_timestamp, uint64_t samplerate,
		const string &custom_name) :
	AnalogBaseSignal(quantity, quantity_flags, unit, parent_channel, custom_name),
	signal_start_timestamp_(signal_start_timestamp),
	actual_samplerate_(samplerate),
	last_timestamp_(0.)
{
	/*
	qWarning() << "Init analog scope signal " << display_name()
		<< ", signal_start_timestamp_ = "
		<< util::format_time_date(signal_start_timestamp_);
	*/

	qWarning() << "AnalogScopeSignal::AnalogScopeSignal(): samplerate = " << samplerate;
	if (samplerate > 0) { // TODO: else throw ex
		time_stride_ = 1 / (double)samplerate;
		qWarning() << "AnalogScopeSignal::AnalogScopeSignal(): time_stride_ = " << time_stride_;
	}
}

void AnalogScopeSignal::clear()
{
	// TODO: mutex
	data_->clear();
	sample_count_ = 0;

	Q_EMIT samples_cleared();
}

analog_scope_sample_t AnalogScopeSignal::get_sample(size_t pos) const
{
	// TODO: retrun reference (&double)? See get_value_at_timestamp()

	//qWarning() << "AnalogScopeSignal::get_sample(" << pos
	//	<< "): sample_count_ = " << sample_count_;

	if (pos >= sample_count_)
		return make_pair(0., 0.);

	double timestamp = time_stride_ * pos;
	//qWarning() << "AnalogScopeSignal::get_sample(" << pos
	//	<< "): sample = " << timestamp << ", " << data_->at(pos);
	return make_pair(timestamp, data_->at(pos));

}

analog_scope_sample_t AnalogScopeSignal::get_last_sample() const
{
	// TODO: retrun reference (&double)? See get_value_at_timestamp()
	if (sample_count_ == 0)
		return make_pair(0., 0.);

	size_t pos = sample_count_ - 1;
	double timestamp = time_stride_ * pos;
	return make_pair(timestamp, data_->at(pos));
}

void AnalogScopeSignal::push_samples(void *data,
	uint64_t samples, double timestamp, uint64_t samplerate, size_t unit_size,
	int digits, int decimal_places)
{
	/*
	//lock_guard<recursive_mutex> lock(mutex_);

	(void)timestamp;
	// TODO: Connect signal for sample rates changes and recalc stride.

	double dsample;
	uint64_t pos = 0;
	while (pos < samples) {
		if (unit_size == size_of_float_)
			dsample = (double) ((float *)data)[pos];
		else if (unit_size == size_of_double_)
			dsample = ((double *)data)[pos];

		// TODO: Mutex?
		if (min_value_ > dsample)
			min_value_ = dsample;
		if (max_value_ < dsample)
			max_value_ = dsample;

		// TODO: Limit memory!
		data_->push_back(dsample);

		last_timestamp_ += time_stride_;
		++pos;
		++sample_count_;
	}

	//last_timestamp_ += time_stride_ * sample_count_;
	last_value_ = dsample;
	Q_EMIT sample_appended();

	bool digits_chngd = false;
	if (digits != digits_) {
		digits_ = digits;
		digits_chngd = true;
	}
	if (decimal_places != decimal_places_) {
		decimal_places_ = decimal_places;
		digits_chngd = true;
	}
	if (digits_chngd)
		Q_EMIT digits_changed(digits_, decimal_places_);
	*/


	(void)timestamp;
	(void)unit_size;
	(void)digits;
	(void)decimal_places;


	// If there is no actual_segmant_, created one now. I.e. this is the first
	// packet the sweep containing this segment.
	if (actual_segment_ == nullptr) {
		//bool sweep_beginning = true;

		// Create a segment, keep it in the maps of channels
		actual_segment_ = make_shared<data::AnalogSegment>(
			*this, segments_.size(), samplerate);

		connect(actual_segment_.get(), &sv::data::AnalogSegment::samples_added,
			this, &AnalogScopeSignal::samples_added);

		qWarning() << "AnalogScopeSignal::push_samples(): Create new AnalogSegment "
			<< actual_segment_->id() <<  " for signal " << display_name();

		// Push the segment into the analog data.
		segments_.push_back(actual_segment_);

		Q_EMIT segment_added(actual_segment_->id());
	}

	if (samplerate > 0) { // TODO: else throw ex
		time_stride_ = 1 / (double)samplerate;
		//qWarning() << "AnalogScopeSignal::push_samples(): time_stride_ = " << time_stride_;
	}

	actual_segment_->append_interleaved_samples((float *)data, samples, 1);
}

shared_ptr<AnalogSegment> AnalogScopeSignal::get_last_segment() const
{
	return segments_.back();
}

shared_ptr<AnalogSegment> AnalogScopeSignal::get_segment(uint32_t segment_id) const
{
	return segments_.at(segment_id);
}

void AnalogScopeSignal::complete_actual_segment()
{
	if (actual_segment_ == nullptr)
		return;

	disconnect(actual_segment_.get(), &sv::data::AnalogSegment::samples_added,
		this, &AnalogScopeSignal::samples_added);

	actual_segment_->set_complete();
	actual_segment_ = nullptr;
}

double AnalogScopeSignal::actual_time_stride() const
{
	return time_stride_;
}

double AnalogScopeSignal::signal_start_timestamp() const
{
	return signal_start_timestamp_;
}

double AnalogScopeSignal::first_timestamp() const
{
	return 0.;
}

double AnalogScopeSignal::last_timestamp() const
{
	return last_timestamp_;
}

} // namespace data
} // namespace sv
