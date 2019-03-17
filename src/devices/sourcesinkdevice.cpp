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

#include <algorithm>
#include <cassert>
#include <string>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "sourcesinkdevice.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/dividechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/channels/integratechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/channels/multiplysschannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/configurable.hpp"

using std::static_pointer_cast;
using std::string;

namespace sv {
namespace devices {

SourceSinkDevice::SourceSinkDevice(
		const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device) :
	HardwareDevice(sr_context, sr_device)
{
}

void SourceSinkDevice::init_channels()
{
	HardwareDevice::init_channels();

	// Preinitialize known fixed channels with a signal
	for (const auto &chg_name_channels_pair : channel_group_map_) {
		for (const auto &channel : chg_name_channels_pair.second) {
			if (channel->type() != channels::ChannelType::AnalogChannel)
				continue;

			// TODO: preinit with channel.meaning.mq, ...
			//       (must be implemented in sigrok)
			bool init = false;
			data::Quantity quantity;
			set<data::QuantityFlag> quantity_flags;
			data::Unit unit;
			if (channel->name().rfind("V", 0) == 0) {
				quantity = data::Quantity::Voltage;
				// TODO: Set AC for AC Sources
				quantity_flags.insert(data::QuantityFlag::DC);
				unit = data::Unit::Volt;
				init = true;
			}
			else if (channel->name().rfind("I", 0) == 0) {
				quantity = data::Quantity::Current;
				// TODO: Set AC for AC Sources
				quantity_flags.insert(data::QuantityFlag::DC);
				unit = data::Unit::Ampere;
				init = true;
			}
			else if (channel->name().rfind("P", 0) == 0) {
				quantity = data::Quantity::Power;
				unit = data::Unit::Watt;
				init = true;
			}
			else if (channel->name().rfind("F", 0) == 0) {
				quantity = data::Quantity::Frequency;
				unit = data::Unit::Hertz;
				init = true;
			}

			if (init) {
				auto hw_channel =
					static_pointer_cast<channels::HardwareChannel>(channel);
				hw_channel->set_fixed_signal(true);
				hw_channel->add_signal(quantity, quantity_flags, unit);
			}
		}

		// Math Channels
		string chg_name = chg_name_channels_pair.first;
		set<string> chg_names { chg_name };
		shared_ptr<data::AnalogSignal> voltage_signal;
		shared_ptr<data::AnalogSignal> current_signal;
		shared_ptr<data::AnalogSignal> power_signal;
		for (const auto &channel : chg_name_channels_pair.second) {
			if (!channel->fixed_signal())
				continue;
			auto signal = channel->actual_signal();
			if (signal->quantity() == data::Quantity::Voltage)
				voltage_signal = static_pointer_cast<data::AnalogSignal>(signal);
			else if (signal->quantity() == data::Quantity::Current)
				current_signal = static_pointer_cast<data::AnalogSignal>(signal);
			else if (signal->quantity() == data::Quantity::Power)
				power_signal = static_pointer_cast<data::AnalogSignal>(signal);
		}

		// Create power channel
		if (voltage_signal && current_signal && !power_signal) {
			shared_ptr<channels::MultiplySSChannel> power_channel =
				make_shared<channels::MultiplySSChannel>(
					data::Quantity::Power,
					set<data::QuantityFlag>(),
					data::Unit::Watt,
					voltage_signal, current_signal,
					shared_from_this(),
					chg_names, tr("P").toStdString(),
					aquisition_start_timestamp_);
			BaseDevice::add_math_channel(power_channel, chg_name);
			power_signal = static_pointer_cast<data::AnalogSignal>(
				power_channel->actual_signal());
		}

		// Create resistance channel
		if (voltage_signal && current_signal) {
			shared_ptr<channels::DivideChannel> resistance_channel =
				make_shared<channels::DivideChannel>(
					data::Quantity::Resistance,
					set<data::QuantityFlag>(),
					data::Unit::Ohm,
					voltage_signal, current_signal,
					shared_from_this(),
					chg_names, tr("R").toStdString(),
					aquisition_start_timestamp_);
			BaseDevice::add_math_channel(resistance_channel, chg_name);
		}

		// Create Wh channel
		if (power_signal) {
			shared_ptr<channels::IntegrateChannel> wh_channel =
				make_shared<channels::IntegrateChannel>(
					data::Quantity::Work,
					set<data::QuantityFlag>(),
					data::Unit::WattHour,
					power_signal,
					shared_from_this(),
					chg_names, tr("Wh").toStdString(),
					aquisition_start_timestamp_);
			BaseDevice::add_math_channel(wh_channel, chg_name);
		}

		// Create Ah channel
		if (current_signal) {
			shared_ptr<channels::IntegrateChannel> ah_channel =
				make_shared<channels::IntegrateChannel>(
					data::Quantity::ElectricCharge,
					set<data::QuantityFlag>(),
					data::Unit::AmpereHour,
					current_signal,
					shared_from_this(),
					chg_names, tr("Ah").toStdString(),
					aquisition_start_timestamp_);
			BaseDevice::add_math_channel(ah_channel, chg_name);
		}
	}
}

} // namespace devices
} // namespace sv
