/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

namespace sv {
namespace devices {

SourceSinkDevice::SourceSinkDevice(
		const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device) :
	HardwareDevice(sr_context, sr_device)
{
	// Set options for different device types
	const auto sr_keys = sr_device->driver()->config_keys();
	if (sr_keys.count(sigrok::ConfigKey::POWER_SUPPLY))
		device_type_ = DeviceType::PowerSupply;
	else if (sr_keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD))
		device_type_ = DeviceType::ElectronicLoad;
	else
		assert("Unknown device");
}

void SourceSinkDevice::init_channels()
{
	HardwareDevice::init_channels();

	// Preinitialize known fixed channels with a signal
	for (auto chg_name_channels_pair : channel_group_name_map_) {
		for (auto channel : chg_name_channels_pair.second) {
			if (channel->type() != channels::ChannelType::AnalogChannel)
				continue;

			// TODO: preinit with channel.meaning.mq, ...
			//       (must be implemented in sigrok)
			bool init = false;
			data::Quantity quantity;
			set<data::QuantityFlag> quantity_flags;
			data::Unit unit;
			if (channel->name().startsWith("V")) {
				quantity = data::Quantity::Voltage;
				// TODO: Set AC for AC Sources
				quantity_flags.insert(data::QuantityFlag::DC);
				unit = data::Unit::Volt;
				init = true;
			}
			else if (channel->name().startsWith("I")) {
				quantity = data::Quantity::Current;
				// TODO: Set AC for AC Sources
				quantity_flags.insert(data::QuantityFlag::DC);
				unit = data::Unit::Ampere;
				init = true;
			}
			else if (channel->name().startsWith("P")) {
				quantity = data::Quantity::Power;
				unit = data::Unit::Watt;
				init = true;
			}
			else if (channel->name().startsWith("F")) {
				quantity = data::Quantity::Frequency;
				unit = data::Unit::Hertz;
				init = true;
			}

			if (init) {
				auto hw_channel =
					static_pointer_cast<channels::HardwareChannel>(channel);
				hw_channel->set_fixed_signal(true);
				hw_channel->init_signal(quantity, quantity_flags, unit);
			}
		}

		// Math Channels
		QString chg_name = chg_name_channels_pair.first;
		shared_ptr<data::AnalogSignal> voltage_signal;
		shared_ptr<data::AnalogSignal> current_signal;
		shared_ptr<data::AnalogSignal> power_signal;
		for (auto channel : chg_name_channels_pair.second) {
			if (!channel->has_fixed_signal())
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
					shared_from_this(), chg_name, tr("P"),
					aquisition_start_timestamp_);
			power_channel->init_signal();
			power_signal = static_pointer_cast<data::AnalogSignal>(
				power_channel->actual_signal());
			Device::add_channel(power_channel, chg_name);
		}

		// Create resistance channel
		if (voltage_signal && current_signal) {
			shared_ptr<channels::DivideChannel> resistance_channel =
				make_shared<channels::DivideChannel>(
					data::Quantity::Resistance,
					set<data::QuantityFlag>(),
					data::Unit::Ohm,
					voltage_signal, current_signal,
					shared_from_this(), chg_name, tr("R"),
					aquisition_start_timestamp_);
			resistance_channel->init_signal();
			Device::add_channel(resistance_channel, chg_name);
		}

		// Create Wh channel
		if (power_signal) {
			shared_ptr<channels::IntegrateChannel> wh_channel =
				make_shared<channels::IntegrateChannel>(
					data::Quantity::Work,
					set<data::QuantityFlag>(),
					data::Unit::WattHour,
					power_signal,
					shared_from_this(), chg_name, tr("Wh"),
					aquisition_start_timestamp_);
			wh_channel->init_signal();
			Device::add_channel(wh_channel, chg_name);
		}

		// Create Ah channel
		if (current_signal) {
			shared_ptr<channels::IntegrateChannel> ah_channel =
				make_shared<channels::IntegrateChannel>(
					data::Quantity::ElectricCharge,
					set<data::QuantityFlag>(),
					data::Unit::AmpereHour,
					current_signal,
					shared_from_this(), chg_name, tr("Ah"),
					aquisition_start_timestamp_);
			ah_channel->init_signal();
			Device::add_channel(ah_channel, chg_name);
		}
	}
}

void SourceSinkDevice::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	// TODO: Move to devices::Configurable and implement device specific signals

	// TODO: The meta packet is missing the information, to which
	// channel group the config key belongs.
	shared_ptr<devices::Configurable> configurable;
	if (configurables_.size() > 0)
		configurable = configurables_.front();

	for (auto entry : sr_meta->config()) {
		switch (entry.first->id()) {
		case SR_CONF_ENABLED:
			Q_EMIT configurable->enabled_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_VOLTAGE_TARGET:
			Q_EMIT configurable->voltage_target_changed(
				g_variant_get_double(entry.second.gobj()));
			break;
		case SR_CONF_CURRENT_LIMIT:
			Q_EMIT configurable->current_limit_changed(
				g_variant_get_double(entry.second.gobj()));
			break;

		case SR_CONF_OVER_TEMPERATURE_PROTECTION:
			Q_EMIT configurable->otp_enabled_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_TEMPERATURE_PROTECTION_ACTIVE:
			Q_EMIT configurable->otp_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;

		case SR_CONF_OVER_VOLTAGE_PROTECTION_ENABLED:
			Q_EMIT configurable->ovp_enabled_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_VOLTAGE_PROTECTION_ACTIVE:
			Q_EMIT configurable->ovp_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_VOLTAGE_PROTECTION_THRESHOLD:
			Q_EMIT configurable->ovp_threshold_changed(
				g_variant_get_double(entry.second.gobj()));
			break;

		case SR_CONF_OVER_CURRENT_PROTECTION_ENABLED:
			Q_EMIT configurable->ocp_enabled_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_CURRENT_PROTECTION_ACTIVE:
			Q_EMIT configurable->ocp_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_CURRENT_PROTECTION_THRESHOLD:
			Q_EMIT configurable->ocp_threshold_changed(
				g_variant_get_double(entry.second.gobj()));
			break;

		case SR_CONF_UNDER_VOLTAGE_CONDITION:
			Q_EMIT configurable->uvc_enabled_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_UNDER_VOLTAGE_CONDITION_ACTIVE:
			Q_EMIT configurable->uvc_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		/*
		case SR_CONF_UNDER_VOLTAGE_CONDITION_THRESHOLD:
			Q_EMIT uvc_threshold_changed(
				g_variant_get_double(entry.second.gobj()));
			break;
		*/
		default:
			// Unknown metadata is not an error.
			break;
		}
	}
}

} // namespace devices
} // namespace sv
