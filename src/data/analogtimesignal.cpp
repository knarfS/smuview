/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <cassert>
#include <memory>
#include <set>

#include <QDebug>
#include <QString>

#include "analogtimesignal.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"

using std::make_pair;
using std::make_shared;
using std::set;
using std::shared_ptr;
using std::vector;

namespace sv {
namespace data {

AnalogTimeSignal::AnalogTimeSignal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		double signal_start_timestamp) :
	AnalogBaseSignal(quantity, quantity_flags, unit, parent_channel),
	signal_start_timestamp_(signal_start_timestamp),
	last_timestamp_(0.)
{
	qWarning() << "Init analog time signal " << display_name()
		<< ", signal_start_timestamp_ = "
		<< util::format_time_date(signal_start_timestamp_);

	time_ = make_shared<vector<double>>();
}

void AnalogTimeSignal::clear()
{
	// TODO: mutex
	time_->clear();
	data_->clear();
	sample_count_ = 0;

	Q_EMIT samples_cleared();
}

analog_time_sample_t AnalogTimeSignal::get_sample(
	size_t pos, bool relative_time) const
{
	// TODO: retrun reference (&double)? See get_value_at_timestamp()

	//qWarning() << "AnalogSignal::get_sample(" << pos
	//	<< "): sample_count_ = " << sample_count_;

	if (pos < sample_count_) {
		double timestamp = time_->at(pos);
		if (relative_time)
			timestamp -= signal_start_timestamp_;
		//qWarning() << "AnalogSignal::get_sample(" << pos
		//	<< "): sample = " << timestamp << ", " << data_->at(pos);
		return make_pair(timestamp, data_->at(pos));
	}

	return make_pair(0., 0.);
}

analog_time_sample_t AnalogTimeSignal::get_last_sample(bool relative_time) const
{
	// TODO: retrun reference (&double)? See get_value_at_timestamp()
	if (sample_count_ == 0)
		return make_pair(0., 0.);

	size_t pos = sample_count_ - 1;
	double timestamp = time_->at(pos);
	if (relative_time)
		timestamp -= signal_start_timestamp_;
	return make_pair(timestamp, data_->at(pos));
}

bool AnalogTimeSignal::get_value_at_timestamp(
	double timestamp, double &value, bool relative_time) const
{
	if (time_->size() == 0)
		return false;
	if (timestamp < time_->at(0))
		return false;
	if (timestamp > time_->back())
		return false;

	if (relative_time)
		timestamp += signal_start_timestamp_;

	auto lower = std::lower_bound(time_->begin(), time_->end(), timestamp);

	// Check if timestamp and found timestamp match
	if (timestamp == *lower) {
		value = *lower;
		return true;
	}

	size_t lower_pos = lower - time_->begin();

	// Get the previous timestamp for linear interpolation
	if (lower_pos > 0)
		--lower_pos;

	double lower_ts = time_->at(lower_pos);
	double lower_data = data_->at(lower_pos);
	size_t upper_pos = lower_pos + 1;
	double upper_ts = time_->at(upper_pos);

	// Use linear interpolation to get the value beetween time stamps
	double ts_factor = (timestamp - lower_ts) / (upper_ts - lower_ts);
	double data_diff = data_->at(upper_pos) - lower_data;
	double lininter_data = lower_data + (data_diff * ts_factor);

	value = lininter_data;
	return true;
}

void AnalogTimeSignal::push_sample(void *sample, double timestamp,
	size_t unit_size, int digits, int decimal_places)
{
	double dsample = 0.;
	if (unit_size == size_of_float_)
		dsample = (double) *(float *)sample;
	else if (unit_size == size_of_double_)
		dsample = *(double *)sample;

	/*
	qWarning() << "AnalogTimeSignal::push_sample(): " << display_name()
		<< ": sample = " << dsample << " @ " <<  timestamp;
	qWarning() << "AnalogTimeSignal::push_sample(): " << display_name()
		<< ": sample_count_ = " << sample_count_+1;
	*/

	// TODO: Mutex?
	last_timestamp_ = timestamp;
	last_value_ = dsample;
	if (min_value_ > dsample)
		min_value_ = dsample;
	if (max_value_ < dsample)
		max_value_ = dsample;

	/*
	qWarning() << "AnalogTimeSignal::push_sample(): " << display_name()
		<< ": last_timestamp_ = " << last_timestamp_;
	qWarning() << "AnalogTimeSignal::push_sample(): " << display_name()
		<< ": last_value_ = " << last_value_;
	qWarning() << "AnalogTimeSignal::push_sample(): " << display_name()
		<< ": min_value_ = " << min_value_;
	qWarning() << "AnalogTimeSignal::push_sample(): " << display_name()
		<< ": max_value_ = " << max_value_;
	*/

	// TODO: Mutex?
	time_->push_back(timestamp);
	data_->push_back(dsample);
	sample_count_++;
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
}

void AnalogTimeSignal::push_samples(void *data,
	uint64_t samples, double timestamp, uint64_t samplerate, size_t unit_size,
	int digits, int decimal_places)
{
	//lock_guard<recursive_mutex> lock(mutex_);

	double dsample;

	uint64_t pos = 0;
	double time_stride = 0;
	if (samplerate > 0)
		time_stride = 1 / (double)samplerate;

	/*
	if (timestamp < last_timestamp_) {
		qWarning() << "AnalogSignal::push_samples(): samples = " << samples
			<<  ", timestamp < last_timestamp = "
			<< timestamp-signal_start_timestamp_ << " < "
			<< last_timestamp_-signal_start_timestamp_;
	}
	*/

	while (pos < samples) {
		if (unit_size == size_of_float_)
			dsample = (double) ((float *)data)[pos];
		else if (unit_size == size_of_double_)
			dsample = ((double *)data)[pos];

		/*
		qWarning() << "AnalogSignal::push_samples(): " << name_
			<< ": sample = " << dsample << " @ "
			<<  timestamp - signal_start_timestamp_;
		qWarning() << "AnalogSignal::push_samples(): " << name_
			<< ": remaining_samples = " << remaining_samples;
		*/

		// TODO: Mutex?
		if (min_value_ > dsample)
			min_value_ = dsample;
		if (max_value_ < dsample)
			max_value_ = dsample;

		// TODO: Limit memory!
		time_->push_back(timestamp);
		data_->push_back(dsample);

		timestamp += time_stride;
		++pos;
		++sample_count_;
	}

	last_timestamp_ = timestamp - time_stride;
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
}

double AnalogTimeSignal::signal_start_timestamp() const
{
	return signal_start_timestamp_;
}

double AnalogTimeSignal::first_timestamp(bool relative_time) const
{
	if (time_->size() == 0)
		return 0.;

	if (relative_time)
		return time_->front() - signal_start_timestamp_;
	else
		return time_->front();
}

double AnalogTimeSignal::last_timestamp(bool relative_time) const
{
	if (time_->size() == 0)
		return 0.;

	if (relative_time)
		return last_timestamp_ - signal_start_timestamp_;
	else
		return last_timestamp_;

	return last_timestamp_;
}

void AnalogTimeSignal::on_channel_start_timestamp_changed(double timestamp)
{
	signal_start_timestamp_ = timestamp;
	Q_EMIT signal_start_timestamp_changed(timestamp);
}

void AnalogTimeSignal::combine_signals(
	shared_ptr<AnalogTimeSignal> signal1, size_t &signal1_pos,
	shared_ptr<AnalogTimeSignal> signal2, size_t &signal2_pos,
	shared_ptr<vector<double>> time_vector,
	shared_ptr<vector<double>> data1_vector,
	shared_ptr<vector<double>> data2_vector)
{
	// Ignore the first sample(s)
	// TODO: Use last of the ignored samples?
	if (signal1_pos == 0 && signal2_pos == 0) {
		if (signal1->sample_count() <= signal1_pos ||
			signal2->sample_count() <= signal2_pos)
			return;

		double signal1_ts = signal1->get_sample(signal1_pos, false).first;
		double signal2_ts = signal2->get_sample(signal2_pos, false).first;
		if (signal1_ts < signal2_ts) {
			while (signal1_ts < signal2_ts)
				signal1_ts = signal1->get_sample(++signal1_pos, false).first;
		}
		else if (signal1_ts > signal2_ts) {
			while (signal1_ts > signal2_ts)
				signal2_ts = signal2->get_sample(++signal2_pos, false).first;
		}
	}

	while (true) {
		if (signal1->sample_count() <= signal1_pos ||
			signal2->sample_count() <= signal2_pos)
			break;

		/*
		qWarning() << "AnalogSignal::merge_signals(): signal1_size = "
				<< signal1->sample_count() << ", signal1_pos = " << signal1_pos;
		qWarning() << "AnalogSignal::merge_signals(): signal2_size = "
				<< signal2->sample_count() << ", signal2_pos = " << signal2_pos;
		*/

		double time;
		double value1;
		double value2;

		auto signal1_sample = signal1->get_sample(signal1_pos, false);
		auto signal2_sample = signal2->get_sample(signal2_pos, false);
		if (signal1_sample.first == signal2_sample.first) {
			time = signal1_sample.first;
			value1 = signal1_sample.second;
			value2 = signal2_sample.second;
			++signal1_pos;
			++signal2_pos;
		}
		else if (signal1_sample.first < signal2_sample.first &&
			signal2->sample_count() > signal2_pos+1) {

			time = signal1_sample.first;
			value1 = signal1_sample.second;
			if (!signal2->get_value_at_timestamp(time, value2, false))
				return;
			++signal1_pos;
		}
		else if (signal1_sample.first > signal2_sample.first &&
			signal1->sample_count() > signal1_pos+1) {

			time = signal2_sample.first;
			if (!signal1->get_value_at_timestamp(time, value1, false))
				return;
			value2 = signal2_sample.second;
			++signal2_pos;
		}
		else {
			return;
		}

		time_vector->push_back(time);
		data1_vector->push_back(value1);
		data2_vector->push_back(value2);
	}
}

} // namespace data
} // namespace sv
