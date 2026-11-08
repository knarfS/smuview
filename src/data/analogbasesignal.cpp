/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <cmath>
#include <mutex>
#include <set>
#include <string>

#include <QDebug>
#include <QString>

#include "analogbasesignal.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"

using std::set;
using std::string;

namespace sv {
namespace data {

AnalogBaseSignal::AnalogBaseSignal(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		const string &custom_name,
		const string &parent_device_id,
		const string &parent_device_name,
		const set<string> &parent_channel_group_names,
		const string &parent_channel_name) :
	BaseSignal(
		quantity, quantity_flags, unit, custom_name, parent_device_id,
		parent_device_name, parent_channel_group_names, parent_channel_name),
	total_digits_(data::DefaultTotalDigits),
	sr_digits_(data::DefaultSRDigits),
	min_value_(),
	max_value_()
{
}

size_t AnalogBaseSignal::sample_count() const
{
	std::scoped_lock lock(data_mutex_);

	return data_.size();
}

int AnalogBaseSignal::total_digits() const
{
	std::scoped_lock lock(data_mutex_);

	return total_digits_;
}

int AnalogBaseSignal::sr_digits() const
{
	std::scoped_lock lock(data_mutex_);

	return sr_digits_;
}

optional<double> AnalogBaseSignal::last_value() const
{
	std::scoped_lock lock(data_mutex_);

	if (data_.empty())
		return {};
	return data_.back();
}

optional<double> AnalogBaseSignal::min_value() const
{
	std::scoped_lock lock(data_mutex_);

	if (data_.empty())
		return {};
	return min_value_;
}

optional<double> AnalogBaseSignal::max_value() const
{
	std::scoped_lock lock(data_mutex_);

	if (data_.empty())
		return {};
	return max_value_;
}

void AnalogBaseSignal::update_min_max_unlocked(double sample)
{
	// Ignore +/- infinitiy (overflow / underflow) and NaN as max/min values
	if (!std::isfinite(sample))
		return;

	if (!min_value_.has_value() || min_value_ > sample)
		min_value_ = sample;

	if (!max_value_.has_value() || max_value_ < sample)
		max_value_ = sample;
}

bool AnalogBaseSignal::update_digits_unlocked(int total_digits, int sr_digits)
{
	bool digits_changed = false;

	if (total_digits != total_digits_) {
		total_digits_ = total_digits;
		digits_changed = true;
	}

	if (sr_digits != sr_digits_) {
		sr_digits_ = sr_digits;
		digits_changed = true;
	}

	return digits_changed;
}

} // namespace data
} // namespace sv
