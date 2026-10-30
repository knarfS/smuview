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

#include <algorithm>
#include <cassert>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>

#include <QDebug>
#include <QString>

#include "analogtimesignal.hpp"
#include "src/sessionglobals.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"

using std::make_pair;
using std::optional;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {
namespace data {

AnalogTimeSignal::AnalogTimeSignal(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		const string &custom_name) :
	AnalogBaseSignal(quantity, quantity_flags, unit, parent_channel, custom_name)
{
	qInfo() << "Init analog time signal " << display_name();
}

void AnalogTimeSignal::clear()
{
	{
		std::scoped_lock lock(data_mutex_);

		time_.clear();
		data_.clear();
		min_value_ = {};
		max_value_ = {};
	}

	Q_EMIT samples_cleared();
}

analog_time_sample_t AnalogTimeSignal::get_sample(
	size_t pos, bool relative_time) const
{
	std::scoped_lock lock(data_mutex_);

	if (pos >= data_.size())
		return {};

	double timestamp = time_.at(pos);
	if (relative_time)
		timestamp -= sv::sessionglobals::session_start_timestamp;
	return make_pair(timestamp, data_.at(pos));
}

analog_time_sample_t AnalogTimeSignal::get_last_sample(bool relative_time) const
{
	std::scoped_lock lock(data_mutex_);

	if (data_.size() == 0)
		return {};

	size_t pos = data_.size() - 1;
	double timestamp = relative_time
		? time_.at(pos) - sv::sessionglobals::session_start_timestamp
		: time_.at(pos);
	return make_pair(timestamp, data_.at(pos));
}

optional<double> AnalogTimeSignal::get_value_at_timestamp(
	double timestamp, bool relative_time) const
{
	std::scoped_lock lock(data_mutex_);

	if (relative_time)
		timestamp += sv::sessionglobals::session_start_timestamp;

	if (time_.empty())
		return {};
	if (timestamp < time_.at(0))
		return {};
	if (timestamp > time_.back())
		return {};

	auto lower = std::lower_bound(time_.begin(), time_.end(), timestamp);
	size_t lower_pos = lower - time_.begin();

	// Check if timestamp and found timestamp match
	if (timestamp == *lower)
		return data_.at(lower_pos);

	// Get the previous timestamp for linear interpolation
	if (lower_pos > 0)
		--lower_pos;

	double lower_ts = time_.at(lower_pos);
	double lower_data = data_.at(lower_pos);
	size_t upper_pos = lower_pos + 1;
	double upper_ts = time_.at(upper_pos);

	// Use linear interpolation to get the value beetween time stamps
	double ts_factor = (timestamp - lower_ts) / (upper_ts - lower_ts);
	double data_diff = data_.at(upper_pos) - lower_data;
	double lininter_data = lower_data + (data_diff * ts_factor);

	return lininter_data;
}

void AnalogTimeSignal::push_sample(double sample, double timestamp,
	int total_digits, int sr_digits)
{
	bool digits_chngd = false;

	// Copy the digits vars inside the locked scope to avoid race conditions
	int total_digits_copy;
	int sr_digits_copy;

	{
		std::scoped_lock lock(data_mutex_);

		update_min_max_unlocked(sample);
		digits_chngd = update_digits_unlocked(total_digits, sr_digits);

		total_digits_copy = total_digits_;
		sr_digits_copy = sr_digits_;

		time_.push_back(timestamp);
		data_.push_back(sample);
	}

	Q_EMIT sample_appended();
	if (digits_chngd)
		Q_EMIT digits_changed(total_digits_copy, sr_digits_copy);
}

void AnalogTimeSignal::push_samples(const vector<double> &samples,
	double timestamp, uint64_t samplerate, int total_digits, int sr_digits)
{
	if (samples.empty())
		return;

	double time_stride = 0.;
	if (samplerate > 0)
		time_stride = 1 / (double)samplerate;

	bool digits_chngd = false;

	// Copy the digits vars inside the locked scope to avoid race conditions
	int total_digits_copy;
	int sr_digits_copy;

	{
		std::scoped_lock lock(data_mutex_);

		double start_timestamp = timestamp;
		time_.reserve(time_.size() + samples.size());
		data_.reserve(data_.size() + samples.size());

		for (size_t i = 0; i < samples.size(); i++) {
			update_min_max_unlocked(samples.at(i));

			// Calculate timestamp from start_timestamp and not just add
			// time_stride, so no floating point rounding error accumulates
			time_.push_back(start_timestamp + i * time_stride);
			data_.push_back(samples.at(i));
		}

		digits_chngd = update_digits_unlocked(total_digits, sr_digits);
		total_digits_copy = total_digits_;
		sr_digits_copy = sr_digits_;
	}

	Q_EMIT sample_appended();
	if (digits_chngd)
		Q_EMIT digits_changed(total_digits_copy, sr_digits_copy);
}

optional<double> AnalogTimeSignal::first_timestamp(bool relative_time) const
{
	std::scoped_lock lock(data_mutex_);

	if (time_.empty())
		return {};

	return relative_time
		? time_.front() - sv::sessionglobals::session_start_timestamp
		: time_.front();
}

optional<double> AnalogTimeSignal::last_timestamp(bool relative_time) const
{
	std::scoped_lock lock(data_mutex_);

	if (time_.empty())
		return {};

	return relative_time
		? time_.back() - sv::sessionglobals::session_start_timestamp
		: time_.back();
}

optional<AnalogTimeSignalBoundaries> AnalogTimeSignal::get_boundaries(
	bool relative_time) const
{
	std::scoped_lock lock(data_mutex_);

	if (time_.empty())
		return {};

	return AnalogTimeSignalBoundaries {
		relative_time
			? time_.front() - sv::sessionglobals::session_start_timestamp
			: time_.front(),
		relative_time
			? time_.back() - sv::sessionglobals::session_start_timestamp
			: time_.back(),
		min_value_.value(),
		max_value_.value()
	};
}

void AnalogTimeSignal::combine_signals(
	shared_ptr<AnalogTimeSignal> signal1, size_t &signal1_pos,
	shared_ptr<AnalogTimeSignal> signal2, size_t &signal2_pos,
	shared_ptr<vector<double>> time_vector,
	shared_ptr<vector<double>> data1_vector,
	shared_ptr<vector<double>> data2_vector)
{
	if (signal1 == nullptr)
		return;
	if (signal2 == nullptr)
		return;

	// Find the first adjacent sample(s)
	if (signal1_pos == 0 || signal2_pos == 0) {
		auto signal1_sample = signal1->get_sample(signal1_pos, false);
		auto signal2_sample = signal2->get_sample(signal2_pos, false);
		if (!signal1_sample.has_value() || !signal2_sample.has_value())
			return;

		double signal1_ts = signal1_sample.value().first;
		double signal2_ts = signal2_sample.value().first;
		if (signal1_ts < signal2_ts) {
			while (signal1_ts < signal2_ts &&
				(signal1_sample = signal1->get_sample(signal1_pos+1, false))) {

				if (!signal1_sample.has_value())
					break;
				signal1_ts = signal1_sample.value().first;
				signal1_pos++;
			}
		}
		else if (signal1_ts > signal2_ts) {
			while (signal1_ts > signal2_ts &&
				(signal2_sample = signal2->get_sample(signal2_pos+1, false))) {

				if (!signal2_sample.has_value())
					break;
				signal2_ts = signal2_sample.value().first;
				signal2_pos++;
			}
		}
	}

	auto signal1_sample = signal1->get_sample(signal1_pos, false);
	auto signal2_sample = signal2->get_sample(signal2_pos, false);
	while (true) {
		if (!signal1_sample.has_value() || !signal2_sample.has_value())
			return;

		auto signal1_ts = signal1_sample.value().first;
		auto signal2_ts = signal2_sample.value().first;

		double time;
		optional<double> value1;
		optional<double> value2;

		if (signal1_ts == signal2_ts) {
			time = signal1_ts;
			value1 = signal1_sample.value().second;
			value2 = signal2_sample.value().second;
			signal1_sample = signal1->get_sample(++signal1_pos, false);
			signal2_sample = signal2->get_sample(++signal2_pos, false);
		}
		else if (signal1_ts < signal2_ts) {
			time = signal1_ts;
			value2 = signal2->get_value_at_timestamp(time, false);
			if (!value2.has_value())
				return;
			value1 = signal1_sample.value().second;
			signal1_sample = signal1->get_sample(++signal1_pos, false);
		}
		else if (signal1_ts > signal2_ts) {
			time = signal2_ts;
			value1 = signal1->get_value_at_timestamp(time, false);
			if (!value1.has_value())
				return;
			value2 = signal2_sample.value().second;
			signal2_sample = signal2->get_sample(++signal2_pos, false);
		}
		else {
			return;
		}

		time_vector->push_back(time);
		data1_vector->push_back(value1.value());
		data2_vector->push_back(value2.value());
	}
}

} // namespace data
} // namespace sv
