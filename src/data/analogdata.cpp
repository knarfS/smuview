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

#include "analogdata.hpp"
#include "src/util.hpp"

using std::make_shared;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;

namespace sv {
namespace data {

AnalogData::AnalogData() : BaseData(),
	sample_count_(0),
	quantity_fixed_(true),
	min_value_(std::numeric_limits<short>::max()),
	max_value_(std::numeric_limits<short>::min())
{
	data_ = make_shared<vector<double>>();
}

void AnalogData::clear()
{
	data_->clear();
	sample_count_ = 0;

	samples_cleared();
}

size_t AnalogData::get_sample_count() const
{
	size_t sample_count = sample_count_;
	//qWarning() << "AnalogData::get_sample_count(): sample_count_ = " << sample_count;
	return sample_count;
}

vector<double> AnalogData::get_samples(size_t start_sample, size_t end_sample) const
{
	assert(start_sample < sample_count_);
	assert(end_sample <= sample_count_);
	assert(start_sample <= end_sample);

	//lock_guard<recursive_mutex> lock(mutex_);

	vector<double>::const_iterator first = data_->begin() + start_sample;
	vector<double>::const_iterator last = data_->begin() + end_sample; // + 1
	vector<double> sub_samples(first, last);

	return sub_samples;
}

double AnalogData::get_sample(size_t pos) const
{
	//assert(pos <= sample_count_);

 	// TODO: retrun reference (&double)?

	if (pos < sample_count_) {
		double sample = data_->at(pos);
		//qWarning() << "AnalogData::get_sample(" << pos << "): sample = " << sample;
		return sample;
	}

	qWarning() << "AnalogData::get_sample(" << pos << "): sample_count_ = " << sample_count_;
	return 0.;
}

void AnalogData::push_sample(void *sample)
{
 	double dsample = (double) *(float*)sample;

	/*
	qWarning() << "AnalogData::push_sample(): sample = " << dsample;
	qWarning() << "AnalogData::push_sample(): sample_count_ = " << sample_count_;
	*/

	last_value_ = dsample;
	if (min_value_ > dsample)
		min_value_ = dsample;
	if (max_value_ < dsample)
		max_value_ = dsample;

	/*
	qWarning() << "AnalogData::push_sample(): last_value_ = " << last_value_;
	qWarning() << "AnalogData::push_sample(): min_value_ = " << min_value_;
	qWarning() << "AnalogData::push_sample(): max_value_ = " << max_value_;
	*/

	data_->push_back(dsample);
	sample_count_++;

	/*
	qWarning() << "AnalogData::push_sample(): sample_count_ = " << sample_count_;
	*/
}

void AnalogData::push_sample(void *sample,
	const sigrok::Quantity *sr_quantity, const sigrok::Unit *sr_unit)
{
	push_sample(sample);

	if (sr_quantity != sr_quantity_) {
		set_quantity(sr_quantity);
		Q_EMIT quantity_changed(quantity_);
	}

	if (sr_unit != sr_unit_) {
		set_unit(sr_unit);
		Q_EMIT unit_changed(unit_);
	}
}

void AnalogData::push_interleaved_samples(/*const*/ float *samples,//void *data,
	size_t sample_count, size_t stride,
	const sigrok::Quantity *sr_quantity, const sigrok::Unit *sr_unit)
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

void AnalogData::set_fixed_quantity(bool fixed)
{
	quantity_fixed_ = fixed;
}

void AnalogData::set_quantity(const sigrok::Quantity *sr_quantity)
{
	sr_quantity_ = sr_quantity;
	quantity_ = util::format_quantity(sr_quantity_);
}

void AnalogData::set_unit(const sigrok::Unit *sr_unit)
{
	sr_unit_ = sr_unit;
	unit_ = util::format_unit(sr_unit_);
}

const QString AnalogData::quantity() const
{
	return quantity_;
}

const QString AnalogData::unit() const
{
	return unit_;
}

double AnalogData::last_value() const
{
	return last_value_;
}

double AnalogData::min_value() const
{
	return min_value_;
}

double AnalogData::max_value() const
{
	return max_value_;
}

} // namespace data
} // namespace sv
