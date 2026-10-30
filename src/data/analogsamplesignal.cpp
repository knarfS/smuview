/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2022 Frank Stettner <frank-stettner@gmx.net>
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
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>

#include <QDebug>
#include <QString>

#include "analogsamplesignal.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"

using std::make_pair;
using std::optional;
using std::set;
using std::shared_ptr;
using std::string;

namespace sv {
namespace data {

AnalogSampleSignal::AnalogSampleSignal(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		const string &custom_name) :
	AnalogBaseSignal(quantity, quantity_flags, unit, parent_channel, custom_name)
{
	qInfo() << "Init analog sample signal " << display_name();
}

void AnalogSampleSignal::clear()
{
	{
		std::scoped_lock lock(data_mutex_);

		pos_.clear();
		data_.clear();
		min_value_ = {};
		max_value_ = {};
	}

	Q_EMIT samples_cleared();
}

analog_pos_sample_t AnalogSampleSignal::get_sample(uint32_t pos) const
{
	std::scoped_lock lock(data_mutex_);

	if (pos >= data_.size())
		return {};

	return make_pair(pos_.at(pos), data_.at(pos));
}

analog_pos_sample_t AnalogSampleSignal::get_last_sample() const
{
	std::scoped_lock lock(data_mutex_);

	if (data_.size() == 0)
		return {};

	size_t pos = data_.size() - 1;
	return make_pair(pos_.at(pos), data_.at(pos));
}


void AnalogSampleSignal::push_sample(double sample, uint32_t pos,
	int total_digits, int sr_digits)
{
	bool digits_chngd = false;

	// Copy the digits vars inside the locked scope to avoid race conditions
	int total_digits_copy;
	int sr_digits_copy;

	{
		std::scoped_lock lock(data_mutex_);

		/*
		qWarning() << "AnalogSampleSignal::push_sample(): " << name_
			<< ": sample = " << dsample << " @ " <<  pos;
		*/

		update_min_max_unlocked(sample);
		digits_chngd = update_digits_unlocked(total_digits, sr_digits);

		total_digits_copy = total_digits_;
		sr_digits_copy = sr_digits_;

		pos_.push_back(pos);
		data_.push_back(sample);
	}

	Q_EMIT sample_appended();
	if (digits_chngd)
		Q_EMIT digits_changed(total_digits_copy, sr_digits_copy);
}

optional<uint32_t> AnalogSampleSignal::first_pos() const
{
	std::scoped_lock lock(data_mutex_);

	if (pos_.empty())
		return {};

	return pos_.front();
}

optional<uint32_t> AnalogSampleSignal::last_pos() const
{
	std::scoped_lock lock(data_mutex_);

	if (pos_.empty())
		return {};

	return pos_.back();
}

} // namespace data
} // namespace sv
