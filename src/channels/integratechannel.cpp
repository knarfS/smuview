/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2019 Frank Stettner <frank-stettner@gmx.net>
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
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<data::AnalogTimeSignal> int_signal,
		shared_ptr<devices::BaseDevice> parent_device,
		set<string> channel_group_names,
		string channel_name,
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

	digits_ = int_signal_->digits();
	decimal_places_ = int_signal_->decimal_places();

	connect(this, SIGNAL(channel_start_timestamp_changed(double)),
		this, SLOT(on_channel_start_timestamp_changed(double)));
	connect(int_signal_.get(), SIGNAL(sample_appended()),
		this, SLOT(on_sample_appended()));
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
	size_t int_signal_sample_count = int_signal_->get_sample_count();
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

} // namespace devices
} // namespace sv
