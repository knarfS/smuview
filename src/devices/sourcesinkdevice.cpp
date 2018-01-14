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
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/channel.hpp"
#include "src/devices/configurable.hpp"

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
		type_ = HardwareDevice::POWER_SUPPLY;
	else if (sr_keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD))
		type_ = HardwareDevice::ELECTRONIC_LOAD;
	else
		assert("Unknown device");

	// Preinitialize known fixed channels with a signal
	for (auto chg_name_channels_pair : channel_group_name_map_) {
		for (auto channel : chg_name_channels_pair.second) {
			bool init = false;
			const sigrok::Quantity *sr_quantity;
			vector<const sigrok::QuantityFlag *> sr_quantity_flags;
			const sigrok::Unit *sr_unit;
			if (channel->internal_name().startsWith("V")) {
				sr_quantity = sigrok::Quantity::VOLTAGE;
				// TODO: Set AC for AC Sources
				sr_quantity_flags.push_back(sigrok::QuantityFlag::DC);
				sr_unit = sigrok::Unit::VOLT;
				init = true;
			}
			else if (channel->internal_name().startsWith("I")) {
				sr_quantity = sigrok::Quantity::CURRENT;
				// TODO: Set AC for AC Sources
				sr_quantity_flags.push_back(sigrok::QuantityFlag::DC);
				sr_unit = sigrok::Unit::AMPERE;
				init = true;
			}
			else if (channel->internal_name().startsWith("P")) {
				sr_quantity = sigrok::Quantity::POWER;
				sr_unit = sigrok::Unit::WATT;
				init = true;
			}
			else if (channel->internal_name().startsWith("F")) {
				sr_quantity = sigrok::Quantity::FREQUENCY;
				sr_unit = sigrok::Unit::HERTZ;
				init = true;
			}

			if (init) {
				channel->set_fixed_signal(true);
				channel->init_signal(sr_quantity, sr_quantity_flags, sr_unit);
			}
		}
	}
}

SourceSinkDevice::~SourceSinkDevice()
{
	close();
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
