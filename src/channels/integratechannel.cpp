/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2022 Frank Stettner <frank-stettner@gmx.net>
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

#include "integratechannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::set;
using std::string;

namespace sv {
namespace channels {

IntegrateChannel::IntegrateChannel(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		shared_ptr<data::AnalogTimeSignal> int_signal,
		shared_ptr<devices::BaseDevice> parent_device,
		const set<string> &channel_group_names,
		const string &channel_name,
		double channel_start_timestamp) :
	MathChannel(quantity, quantity_flags, unit,
		parent_device, channel_group_names, channel_name,
		channel_start_timestamp),
	int_signal_(int_signal),
	next_int_signal_pos_(0),
	last_timestamp_(channel_start_timestamp),
	last_value_(0.)
{
	assert(int_signal_);

	total_digits_ = int_signal_->total_digits();
	sr_digits_ = int_signal_->sr_digits();

	connect(this, &IntegrateChannel::channel_start_timestamp_changed,
		this, &IntegrateChannel::on_channel_start_timestamp_changed);
	connect(int_signal_.get(), &data::AnalogTimeSignal::sample_appended,
		this, &IntegrateChannel::on_sample_appended);
}

void IntegrateChannel::on_channel_start_timestamp_changed(double timestamp)
{
	// TODO: check if already started?
	if (last_timestamp_ < 0)
		last_timestamp_ = timestamp;
}

void IntegrateChannel::on_sample_appended()
{
	// Integrate
	size_t int_signal_sample_count = int_signal_->sample_count();
	while (next_int_signal_pos_ < int_signal_sample_count) {
		auto sample = int_signal_->get_sample(next_int_signal_pos_, false);
		double time = sample.first;
		double elapsed_time_hours = (time - last_timestamp_) / (double)3600;
		double value = last_value_ + (sample.second * elapsed_time_hours);

		push_sample(value, time);

		last_timestamp_ = time;
		last_value_ = value;
		++next_int_signal_pos_;
	}
}

} // namespace channels
} // namespace sv
