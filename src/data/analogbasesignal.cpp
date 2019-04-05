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

#include "analogbasesignal.hpp"
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

AnalogBaseSignal::AnalogBaseSignal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel) :
	BaseSignal(quantity, quantity_flags, unit, parent_channel),
	sample_count_(0),
	digits_(7), // A good start value for digits
	decimal_places_(3), // A good start value for decimal places
	last_value_(0.),
	min_value_(std::numeric_limits<double>::max()),
	max_value_(std::numeric_limits<double>::lowest())
{
	qWarning() << "Init analog base signal " << display_name();
	data_ = make_shared<vector<double>>();
}

size_t AnalogBaseSignal::get_sample_count() const
{
	size_t sample_count = sample_count_;
	//qWarning() << "AnalogBaseSignal::get_sample_count(): sample_count_ = "
	//	<< sample_count;
	return sample_count;
}

/*
analog_time_sample_t AnalogSignal::get_sample(
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
*/

/*
bool AnalogSignal::get_value_at_timestamp(
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
*/

/*
void AnalogSignal::push_sample(void *sample, double timestamp,
	size_t unit_size, int digits, int decimal_places)
{
	double dsample = 0.;
	if (unit_size == size_of_float_)
		dsample = (double) *(float *)sample;
	else if (unit_size == size_of_double_)
		dsample = *(double *)sample;

	/ *
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ": sample = " << dsample << " @ " <<  timestamp;
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ": sample_count_ = " << sample_count_+1;
	* /

	// TODO: Mutex?
	last_timestamp_ = timestamp;
	last_value_ = dsample;
	if (min_value_ > dsample)
		min_value_ = dsample;
	if (max_value_ < dsample)
		max_value_ = dsample;

	/ *
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ":last_timestamp_ = " << last_timestamp_;
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ":last_value_ = " << last_value_;
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ":min_value_ = " << min_value_;
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ":max_value_ = " << max_value_;
	* /

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
*/

int AnalogBaseSignal::digits() const
{
	return digits_;
}

int AnalogBaseSignal::decimal_places() const
{
	return decimal_places_;
}

double AnalogBaseSignal::last_value() const
{
	return last_value_;
}

double AnalogBaseSignal::min_value() const
{
	return min_value_;
}

double AnalogBaseSignal::max_value() const
{
	return max_value_;
}

/*
void AnalogSignal::combine_signals(
	shared_ptr<AnalogSignal> signal1, size_t &signal1_pos,
	shared_ptr<AnalogSignal> signal2, size_t &signal2_pos,
	shared_ptr<vector<double>> time_vector,
	shared_ptr<vector<double>> data1_vector,
	shared_ptr<vector<double>> data2_vector)
{
	// Ignore the first sample(s)
	// TODO: Use last of the ignored samples?
	if (signal1_pos == 0 && signal2_pos == 0) {
		if (signal1->get_sample_count() <= signal1_pos ||
			signal2->get_sample_count() <= signal2_pos)
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
		if (signal1->get_sample_count() <= signal1_pos ||
			signal2->get_sample_count() <= signal2_pos)
			break;

		/ *
		qWarning() << "AnalogSignal::merge_signals(): signal1_size = "
				<< signal1->get_sample_count() << ", signal1_pos = "
				<< signal1_pos;
		qWarning() << "AnalogSignal::merge_signals(): signal2_size = "
				<< signal2->get_sample_count() << ", signal2_pos = "
				<< signal2_pos;
		* /

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
			signal2->get_sample_count() > signal2_pos+1) {

			time = signal1_sample.first;
			value1 = signal1_sample.second;
			if (!signal2->get_value_at_timestamp(time, value2, false))
				return;
			++signal1_pos;
		}
		else if (signal1_sample.first > signal2_sample.first &&
			signal1->get_sample_count() > signal1_pos+1) {

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
*/

} // namespace data
} // namespace sv
