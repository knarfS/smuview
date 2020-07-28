/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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
#include <string>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "sourcesinkdevice.hpp"
#include "src/util.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/dividechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/channels/integratechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/channels/multiplysschannel.hpp"
#include "src/data/analogtimesignal.hpp"
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

void SourceSinkDevice::init_configurables()
{
	HardwareDevice::init_configurables();

	for (const auto &c_pair : configurable_map_) {
		auto configurable = c_pair.second;

		// Check if the device has the config key "Range". If so, the config
		// keys "VoltageTarget" and "CurrentLimit" could have different
		// min/max/step values for each "Range" value!
		if (configurable->property_map().count(ConfigKey::Range) > 0 &&
			configurable->property_map().count(ConfigKey::VoltageTarget) > 0) {

			auto range_property = configurable->property_map()[ConfigKey::Range];
			auto volt_property =
				configurable->property_map()[ConfigKey::VoltageTarget];
			connect(
				range_property.get(), &data::properties::BaseProperty::value_changed,
				volt_property.get(), &data::properties::BaseProperty::list_config);
		}
		if (configurable->property_map().count(ConfigKey::Range) > 0 &&
			configurable->property_map().count(ConfigKey::CurrentLimit) > 0) {

			auto range_property = configurable->property_map()[ConfigKey::Range];
			auto current_property =
				configurable->property_map()[ConfigKey::CurrentLimit];
			connect(
				range_property.get(), &data::properties::BaseProperty::value_changed,
				current_property.get(), &data::properties::BaseProperty::list_config);
		}
	}
}

void SourceSinkDevice::init_channels()
{
	HardwareDevice::init_channels();

	// Preinitialize known fixed channels with a signal
	for (const auto &chg_name_channels_pair : channel_group_map_) {
		string ch_suffix;
		for (const auto &channel : chg_name_channels_pair.second) {
			if (channel->type() != channels::ChannelType::AnalogChannel)
				continue;

			// TODO: Preinit the fixed channels with channel.meaning.mq,
			//       quantity_flags (AC/DC), ...! Must be implemented in sigrok!
			bool init = false;
			data::Quantity quantity;
			set<data::QuantityFlag> quantity_flags;
			data::Unit unit;
			if (util::starts_with(channel->name(), "V")) {
				quantity = data::Quantity::Voltage;
				// TODO: Check for AC/DC when libsigrok preinits the channels!
				quantity_flags.insert(data::QuantityFlag::DC);
				unit = data::Unit::Volt;
				init = true;
				handle_channel_name_suffix(channel->name(), 1, ch_suffix);
			}
			else if (util::starts_with(channel->name(), "I")) {
				quantity = data::Quantity::Current;
				// TODO: Check for AC/DC when libsigrok preinits the channels!
				quantity_flags.insert(data::QuantityFlag::DC);
				unit = data::Unit::Ampere;
				init = true;
				handle_channel_name_suffix(channel->name(), 1, ch_suffix);
			}
			else if (util::starts_with(channel->name(), "P")) {
				quantity = data::Quantity::Power;
				unit = data::Unit::Watt;
				init = true;
				handle_channel_name_suffix(channel->name(), 1, ch_suffix);
			}
			else if (util::starts_with(channel->name(), "R")) {
				quantity = data::Quantity::Resistance;
				unit = data::Unit::Ohm;
				init = true;
				handle_channel_name_suffix(channel->name(), 1, ch_suffix);
			}
			else if (util::starts_with(channel->name(), "F")) {
				quantity = data::Quantity::Frequency;
				unit = data::Unit::Hertz;
				init = true;
				handle_channel_name_suffix(channel->name(), 1, ch_suffix);
			}
			else if (util::starts_with(channel->name(), "Wh") ||
					util::starts_with(channel->name(), "E")) {
				quantity = data::Quantity::Energy;
				unit = data::Unit::WattHour;
				init = true;
				handle_channel_name_suffix(channel->name(), 2, ch_suffix);
			}
			else if (util::starts_with(channel->name(), "Ah")) {
				quantity = data::Quantity::ElectricCharge;
				unit = data::Unit::AmpereHour;
				init = true;
				handle_channel_name_suffix(channel->name(), 2, ch_suffix);
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
		shared_ptr<data::AnalogTimeSignal> voltage_signal;
		shared_ptr<data::AnalogTimeSignal> current_signal;
		shared_ptr<data::AnalogTimeSignal> power_signal;
		shared_ptr<data::AnalogTimeSignal> resistance_signal;
		shared_ptr<data::AnalogTimeSignal> wh_signal;
		shared_ptr<data::AnalogTimeSignal> ah_signal;
		for (const auto &channel : chg_name_channels_pair.second) {
			if (!channel->fixed_signal())
				continue;
			auto signal = channel->actual_signal();
			if (signal->quantity() == data::Quantity::Voltage) {
				voltage_signal =
					static_pointer_cast<data::AnalogTimeSignal>(signal);
			}
			else if (signal->quantity() == data::Quantity::Current) {
				current_signal =
					static_pointer_cast<data::AnalogTimeSignal>(signal);
			}
			else if (signal->quantity() == data::Quantity::Power) {
				power_signal =
					static_pointer_cast<data::AnalogTimeSignal>(signal);
			}
			else if (signal->quantity() == data::Quantity::Resistance) {
				resistance_signal =
					static_pointer_cast<data::AnalogTimeSignal>(signal);
			}
			else if (signal->quantity() == data::Quantity::Energy) {
				wh_signal =
					static_pointer_cast<data::AnalogTimeSignal>(signal);
			}
			else if (signal->quantity() == data::Quantity::ElectricCharge) {
				ah_signal =
					static_pointer_cast<data::AnalogTimeSignal>(signal);
			}
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
					chg_names, "P" + ch_suffix,
					aquisition_start_timestamp_);
			BaseDevice::add_math_channel(power_channel, chg_name);
			power_signal = static_pointer_cast<data::AnalogTimeSignal>(
				power_channel->actual_signal());
		}

		// Create resistance channel
		if (voltage_signal && current_signal && !resistance_signal) {
			shared_ptr<channels::DivideChannel> resistance_channel =
				make_shared<channels::DivideChannel>(
					data::Quantity::Resistance,
					set<data::QuantityFlag>(),
					data::Unit::Ohm,
					voltage_signal, current_signal,
					shared_from_this(),
					chg_names, "R" + ch_suffix,
					aquisition_start_timestamp_);
			BaseDevice::add_math_channel(resistance_channel, chg_name);
		}

		// Create Wh channel
		if (power_signal && !wh_signal) {
			shared_ptr<channels::IntegrateChannel> wh_channel =
				make_shared<channels::IntegrateChannel>(
					data::Quantity::Energy,
					set<data::QuantityFlag>(),
					data::Unit::WattHour,
					power_signal,
					shared_from_this(),
					chg_names, "Wh" + ch_suffix,
					aquisition_start_timestamp_);
			BaseDevice::add_math_channel(wh_channel, chg_name);
		}

		// Create Ah channel
		if (current_signal && !ah_signal) {
			shared_ptr<channels::IntegrateChannel> ah_channel =
				make_shared<channels::IntegrateChannel>(
					data::Quantity::ElectricCharge,
					set<data::QuantityFlag>(),
					data::Unit::AmpereHour,
					current_signal,
					shared_from_this(),
					chg_names, "Ah" + ch_suffix,
					aquisition_start_timestamp_);
			BaseDevice::add_math_channel(ah_channel, chg_name);
		}
	}
}

void SourceSinkDevice::handle_channel_name_suffix(const string &channel_name,
	size_t start_pos, string &channel_suffix)
{
	string tmp_ch_suffix = channel_name.substr(
		start_pos, channel_name.length());

	if (channel_suffix.length() == 0 && tmp_ch_suffix.length() > 0)
		channel_suffix = tmp_ch_suffix;
	else if (channel_suffix != tmp_ch_suffix) {
		qWarning() << "SourceSinkDevice::init_channels(): " <<
			"Channel suffix for channel " <<
			QString::fromStdString(channel_name) <<
			" differs from previous suffix " <<
			QString::fromStdString(channel_suffix);
	}

}

} // namespace devices
} // namespace sv
