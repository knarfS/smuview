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

#include <QDebug>

#include "analog.hpp"

using std::make_shared;
using std::shared_ptr;
using std::vector;

namespace sv {
namespace data {

Analog::Analog() : SignalData(),
	sample_count_(0),
	min_value_(std::numeric_limits<short>::max()),
	max_value_(std::numeric_limits<short>::min())
{
	data_ = make_shared<vector<double>>();
}

void Analog::clear()
{
	data_->clear();
	sample_count_ = 0;

	samples_cleared();
}

size_t Analog::get_sample_count() const
{
	size_t sample_count = sample_count_;
	//qWarning() << "Analog::get_sample_count(): sample_count_ = " << sample_count;
	return sample_count;
}

vector<double> Analog::get_samples(size_t start_sample, size_t end_sample) const
{
	assert(start_sample = 0);
	assert(start_sample < sample_count_);
	assert(end_sample = 0);
	assert(end_sample <= sample_count_);
	assert(start_sample <= end_sample);

	//lock_guard<recursive_mutex> lock(mutex_);

	vector<double>::const_iterator first = data_->begin() + 100000;
	vector<double>::const_iterator last = data_->begin() + 101000;

	vector<double> newVec(first, last);
	return newVec;
}

double Analog::get_sample(size_t pos) const
{
	//assert(pos <= sample_count_);

 	// TODO: retrun reference (&double)?

	if (pos < sample_count_) {
		double sample = data_->at(pos);
		//qWarning() << "Analog::get_sample(" << pos << "): sample = " << sample;
		return sample;
	}

	qWarning() << "Analog::get_sample(" << pos << "): sample_count_ = " << sample_count_;
	return 0.;
}

void Analog::push_sample(void *sample)
{
 	double dsample = (double) *(float*)sample;

	/*
	qWarning() << "Analog::push_sample(): sample = " << dsample;
	qWarning() << "Analog::push_sample(): sample_count_ = " << sample_count_;
	*/

	last_value_ = dsample;
	if (min_value_ > dsample)
		min_value_ = dsample;
	if (max_value_ < dsample)
		max_value_ = dsample;

	/*
	qWarning() << "Analog::push_sample(): last_value_ = " << last_value_;
	qWarning() << "Analog::push_sample(): min_value_ = " << min_value_;
	qWarning() << "Analog::push_sample(): max_value_ = " << max_value_;
	*/

	data_->push_back(dsample);
	sample_count_++;

	//qWarning() << "Analog::push_sample(): sample_count_ = " << sample_count_;
}

double Analog::last_value() const
{
	return last_value_;
}

double Analog::min_value() const
{
	return min_value_;
}

double Analog::max_value() const
{
	return max_value_;
}

} // namespace data
} // namespace sv
