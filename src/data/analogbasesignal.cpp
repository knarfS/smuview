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
#include <memory>
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
using std::shared_ptr;
using std::string;

namespace sv {
namespace data {

AnalogBaseSignal::AnalogBaseSignal(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		const string &custom_name) :
	BaseSignal(quantity, quantity_flags, unit, parent_channel, custom_name),
	total_digits_(data::DefaultTotalDigits),
	sr_digits_(data::DefaultSRDigits),
	min_value_(),
	max_value_()
{
	qWarning() << "Init analog base signal " << display_name();
}

size_t AnalogBaseSignal::sample_count() const
{
	return data_.size();
}

int AnalogBaseSignal::total_digits() const
{
	return total_digits_;
}

int AnalogBaseSignal::sr_digits() const
{
	return sr_digits_;
}

optional<double> AnalogBaseSignal::last_value() const
{
	if (data_.empty())
		return {};
	return data_.back();
}

optional<double> AnalogBaseSignal::min_value() const
{
	if (data_.empty())
		return {};
	return min_value_;
}

optional<double> AnalogBaseSignal::max_value() const
{
	if (data_.empty())
		return {};
	return max_value_;
}

} // namespace data
} // namespace sv
