/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include "analogsamplesignal.hpp"
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

AnalogSampleSignal::AnalogSampleSignal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel) :
	AnalogBaseSignal(quantity, quantity_flags, unit, parent_channel),
	last_pos_(0)
{
	qWarning() << "Init analog sample signal " << display_name();
	pos_ = make_shared<vector<uint32_t>>();
}

void AnalogSampleSignal::clear()
{
	// TODO: mutex
	pos_->clear();
	data_->clear();
	sample_count_ = 0;

	Q_EMIT samples_cleared();
}

analog_pos_sample_t AnalogSampleSignal::get_sample(uint32_t pos) const
{
	// TODO: retrun reference (&double)? See get_value_at_timestamp()

	//qWarning() << "AnalogSampleSignal::get_sample(" << pos
	//	<< "): sample_count_ = " << sample_count_;

	if (pos < sample_count_) {
		//qWarning() << "AnalogSampleSignal::get_sample(" << pos
		//	<< "): value = " << data_->at(pos);
		return make_pair(pos, data_->at(pos));
	}

	return make_pair(0, 0.);
}

void AnalogSampleSignal::push_sample(void *sample, uint32_t pos,
		size_t unit_size, int digits, int decimal_places)
{
	double dsample = 0.;
	if (unit_size == size_of_float_)
		dsample = (double) *(float *)sample;
	else if (unit_size == size_of_double_)
		dsample = *(double *)sample;

	/*
	qWarning() << "AnalogSampleSignal::push_sample(): " << name_
		<< ": sample = " << dsample << " @ " <<  pos;
	qWarning() << "AnalogSampleSignal::push_sample(): " << name_
		<< ": sample_count_ = " << sample_count_+1;
	*/

	// TODO: Mutex?
	last_pos_ = pos;
	last_value_ = dsample;
	if (min_value_ > dsample)
		min_value_ = dsample;
	// Ignore infinitiy (overflow) as max value.
	if (max_value_ < dsample &&
		dsample != std::numeric_limits<double>::infinity()) {

		max_value_ = dsample;
	}

	/*
	qWarning() << "AnalogSampleSignal::push_sample(): " << name_
		<< ":last_pos_ = " << last_pos_;
	qWarning() << "AnalogSampleSignal::push_sample(): " << name_
		<< ":last_value_ = " << last_value_;
	qWarning() << "AnalogSampleSignal::push_sample(): " << name_
		<< ":min_value_ = " << min_value_;
	qWarning() << "AnalogSampleSignal::push_sample(): " << name_
		<< ":max_value_ = " << max_value_;
	*/

	// TODO: Mutex?
	pos_->push_back(pos);
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

uint32_t AnalogSampleSignal::first_pos() const
{
	if (pos_->size() == 0)
		return 0;

	return pos_->front();
}

uint32_t AnalogSampleSignal::last_pos() const
{
	if (pos_->size() == 0)
		return 0;

	return last_pos_;
}

/*
void AnalogSampleSignal::combine_signals(
	shared_ptr<AnalogSampleSignal> signal1, size_t &signal1_pos,
	shared_ptr<AnalogSampleSignal> signal2, size_t &signal2_pos,
	shared_ptr<vector<uint32_t>> pos_vector,
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
		qWarning() << "AnalogSampleSignal::merge_signals(): signal1_size = "
				<< signal1->get_sample_count() << ", signal1_pos = "
				<< signal1_pos;
		qWarning() << "AnalogSampleSignal::merge_signals(): signal2_size = "
				<< signal2->get_sample_count() << ", signal2_pos = "
				<< signal2_pos;
		* /

		double time;
		double value1;
		double value2;

		sample_t signal1_sample = signal1->get_sample(signal1_pos, false);
		sample_t signal2_sample = signal2->get_sample(signal2_pos, false);
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
