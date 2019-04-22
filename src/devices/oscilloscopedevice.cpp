/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "oscilloscopedevice.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"

using std::static_pointer_cast;

namespace sv {
namespace devices {

OscilloscopeDevice::OscilloscopeDevice(
		const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device) :
	HardwareDevice(sr_context, sr_device)
{
}

void OscilloscopeDevice::init_channels()
{
	HardwareDevice::init_channels();

	for (const auto &chg_name_channels_pair : channel_group_map_) {
		for (const auto &channel : chg_name_channels_pair.second) {
			if (channel->type() != channels::ChannelType::AnalogChannel)
				continue;

			// TODO: preinit with channel.meaning.mq, ...
			//       (must be implemented in sigrok)
			data::Quantity quantity = data::Quantity::Voltage;
			set<data::QuantityFlag> quantity_flags;
			data::Unit unit = data::Unit::Volt;

			auto hw_channel =
				static_pointer_cast<channels::HardwareChannel>(channel);
			hw_channel->add_signal(quantity, quantity_flags, unit);
		}
	}
}

} // namespace devices
} // namespace sv
