/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>
#include <QString>

#include "analogsignal.hpp"
#include "src/util.hpp"

using std::make_pair;
using std::make_shared;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;

namespace sv {
namespace data {

AnalogSignal::AnalogSignal(
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit,
		const QString device_name,
		const QString channel_group_name, const QString channel_name,
		double signal_start_timestamp) :
	BaseSignal(sr_quantity, sr_quantity_flags, sr_unit,
		device_name, channel_group_name, channel_name),
	sample_count_(0),
	digits_(7), // A good start value for digits
	decimal_places_(-1), // A good start value for decimal places
	signal_start_timestamp_(signal_start_timestamp),
	last_timestamp_(0.),
	last_value_(0.),
	min_value_(std::numeric_limits<double>::max()),
	max_value_(std::numeric_limits<double>::lowest())
{
	qWarning()
		<< "Init analog signal " << name_
		<< ", signal_start_timestamp_ = "
		<< util::format_time_date(signal_start_timestamp_);

	time_ = make_shared<vector<double>>();
	data_ = make_shared<vector<double>>();
}

void AnalogSignal::clear()
{
	data_->clear();
	sample_count_ = 0;

	samples_cleared();
}

size_t AnalogSignal::get_sample_count() const
{
	size_t sample_count = sample_count_;
	//qWarning() << "AnalogSignal::get_sample_count(): sample_count_ = "
	//	<< sample_count;
	return sample_count;
}

vector<double> AnalogSignal::get_samples(
	size_t start_sample, size_t end_sample) const
{
	assert(start_sample < sample_count_);
	assert(end_sample <= sample_count_);
	assert(start_sample <= end_sample);

	// TODO: lock_guard<recursive_mutex> lock(mutex_);
	// TODO: relative time

	vector<double>::const_iterator first = data_->begin() + start_sample;
	vector<double>::const_iterator last = data_->begin() + end_sample; // + 1
	vector<double> sub_samples(first, last);

	return sub_samples;
}

sample_t AnalogSignal::get_sample(size_t pos, bool is_relative_time) const
{
	//assert(pos <= sample_count_);

 	// TODO: retrun reference (&double)?

	//qWarning() << "AnalogSignal::get_sample(" << pos
	//	<< "): sample_count_ = " << sample_count_;

	if (pos < sample_count_) {
		double timestamp = time_->at(pos);
		if (is_relative_time)
			timestamp -= signal_start_timestamp_;
		//qWarning() << "AnalogSignal::get_sample(" << pos
		//	<< "): sample = " << timestamp << ", " << data_->at(pos);
		return make_pair(timestamp, data_->at(pos));
	}

	return make_pair(0., 0.);
}

void AnalogSignal::push_sample(void *sample, double timestamp,
		int digits, int decimal_places)
{
	// TODO: double / float
 	double dsample = (double) *(float*)sample;

	/*
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ": sample = " << dsample << " @ " <<  timestamp;
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ":sample_count_ = " << sample_count_+1;
	*/

	// TODO: Mutex?
	last_timestamp_ = timestamp;
	last_value_ = dsample;
	if (min_value_ > dsample)
		min_value_ = dsample;
	if (max_value_ < dsample)
		max_value_ = dsample;

	/*
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ":last_timestamp_ = " << last_timestamp_;
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ":last_value_ = " << last_value_;
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ":min_value_ = " << min_value_;
	qWarning() << "AnalogSignal::push_sample(): " << name_
		<< ":max_value_ = " << max_value_;
	*/

	// TODO: Mutex?
	time_->push_back(timestamp);
	data_->push_back(dsample);
	sample_count_++;
	Q_EMIT sample_added();

	if (digits != digits_) {
		digits_ = digits;
		Q_EMIT digits_changed(digits_);
	}

	if (decimal_places != decimal_places_) {
		decimal_places_ = decimal_places;
		Q_EMIT decimal_places_changed(decimal_places_);
	}
}

/*
void AnalogSignal::push_interleaved_samples(/ *const* / float *samples,//void *data,
	size_t sample_count, size_t stride, const sigrok::Unit *sr_unit)
{
	//assert(unit_size_ == sizeof(float));

	if (sr_quantity != sr_quantity_) {
		set_quantity(sr_quantity);
		Q_EMIT quantity_changed(quantity_);
	}

	if (sr_unit != sr_unit_) {
		set_unit(sr_unit);
		Q_EMIT unit_changed(unit_);
	}

	//lock_guard<recursive_mutex> lock(mutex_);

	//uint64_t prev_sample_count = sample_count_;

	// Deinterleave the samples and add them
	for (uint32_t i = 0; i < sample_count; i++) {
		push_sample(samples);
		samples += stride;
	}
}
*/

int AnalogSignal::digits() const
{
	return digits_;
}

int AnalogSignal::decimal_places() const
{
	return decimal_places_;
}

double AnalogSignal::signal_start_timestamp() const
{
	return signal_start_timestamp_;
}

double AnalogSignal::first_timestamp(bool relative_time) const
{
	if (time_->size() == 0)
		return 0.;

	if (relative_time)
		return time_->front() - signal_start_timestamp_;
	else
		return time_->front();
}

double AnalogSignal::last_timestamp(bool relative_time) const
{
	if (time_->size() == 0)
		return 0.;

	if (relative_time)
		return last_timestamp_ - signal_start_timestamp_;
	else
		return last_timestamp_;

	return last_timestamp_;
}

double AnalogSignal::last_value() const
{
	return last_value_;
}

double AnalogSignal::min_value() const
{
	return min_value_;
}

double AnalogSignal::max_value() const
{
	return max_value_;
}

void AnalogSignal::on_channel_start_timestamp_changed(double timestamp)
{
	signal_start_timestamp_ = timestamp;
	Q_EMIT signal_start_timestamp_changed(timestamp);
}

} // namespace data
} // namespace sv
