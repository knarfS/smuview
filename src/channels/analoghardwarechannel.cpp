/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>
#include <set>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "analoghardwarechannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/devices/basedevice.hpp"

using std::make_shared;
using std::set;

namespace sv {
namespace channels {

AnalogHardwareChannel::AnalogHardwareChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		set<string> channel_group_names,
		double channel_start_timestamp) :
	HardwareChannel(sr_channel, parent_device, channel_group_names,
		channel_start_timestamp)
{
	channel_type_ = ChannelType::AnalogChannel;

	qWarning() << "AnalogHardwareChannel::AnalogHardwareChannel(): name = " <<
		display_name();
}

shared_ptr<data::BaseSignal> AnalogHardwareChannel::add_signal(
	data::Quantity quantity,
	set<data::QuantityFlag> quantity_flags,
	data::Unit unit)
{
	/*
	 * TODO: Remove shared_from_this() / (channel pointer in signal), so that
	 *       "add_signal()" can be called from MathChannel ctor.
	 */
	auto signal = make_shared<data::AnalogTimeSignal>(
		quantity, quantity_flags, unit,
		shared_from_this(), channel_start_timestamp_); // TODO: timestamp

	BaseChannel::add_signal(signal);

	return signal;
}

void AnalogHardwareChannel::on_frame_begin(double timestamp, uint64_t samplerate)
{
	(void)timestamp;
	(void)samplerate;
}

} // namespace channels
} // namespace sv
