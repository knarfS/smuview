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
#include "src/devices/configurable.hpp"

namespace sv {
namespace devices {

SourceSinkDevice::SourceSinkDevice(
		const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device) :
	HardwareDevice(sr_context, sr_device)
{
	/* When multiple channels data is send within a frame, they have common
	 * time stamps.
	 *
	 * TODO: Implement a common time base per channel group. When the "one
	 * command - multiple return values" feature is done, a frame contains only
	 * one channel group.
	 * TODO: The common time data should be detected when a frame starts.
	 * Maybe use one vector per channel and don't share them.
	 */
	shared_ptr<vector<double>> common_time_data;

	// Set options for different device types
	const auto sr_keys = sr_device->driver()->config_keys();
	if (sr_keys.count(sigrok::ConfigKey::POWER_SUPPLY)) {
		type_ = HardwareDevice::POWER_SUPPLY;
		common_time_data = nullptr;
	}
	else if (sr_keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD)) {
		type_ = HardwareDevice::ELECTRONIC_LOAD;
		common_time_data = make_shared<vector<double>>(); // TODO
	}
	else {
		type_ = HardwareDevice::UNKNOWN;
		assert("Unknown device");
	}

	// TODO: move to hw device ctor, when common_time_data is fixed
	// Init signals from Sigrok Channel Groups
	map<string, shared_ptr<sigrok::ChannelGroup>> sr_channel_groups =
		sr_device_->channel_groups();
	if (sr_channel_groups.size() > 0) {
		for (auto sr_cg_pair : sr_channel_groups) {
			shared_ptr<sigrok::ChannelGroup> sr_cg = sr_cg_pair.second;
			QString cg_name = QString::fromStdString(sr_cg->name());
			for (auto sr_channel : sr_cg->channels()) {
				init_signal(sr_channel, cg_name, common_time_data);
			}
		}
	}

	// Init signals that are not in a channel group
	vector<shared_ptr<sigrok::Channel>> sr_channels = sr_device_->channels();
	for (auto sr_channel : sr_channels) {
		if (sr_channel_signal_map_.count(sr_channel) > 0)
			continue;
		// TODO: sr_channel must not have a signal (see Digi35)....
		init_signal(sr_channel, QString(""), common_time_data);
	}
}

SourceSinkDevice::~SourceSinkDevice()
{
	close();
}

void SourceSinkDevice::init_signal(
	shared_ptr<sigrok::Channel> sr_channel,
	QString channel_group_name,
	shared_ptr<vector<double>> common_time_data)
{
	if (sr_channel->type()->id() != SR_CHANNEL_ANALOG)
		return;

	//lock_guard<recursive_mutex> lock(data_mutex_);

	const sigrok::Quantity *sr_quantity;
	QString signal_name = QString::fromStdString(sr_channel->name());
	if (signal_name.startsWith("V"))
		sr_quantity = sigrok::Quantity::VOLTAGE;
	else if (signal_name.startsWith("I"))
		sr_quantity = sigrok::Quantity::CURRENT;
	else if (signal_name.startsWith("P"))
		sr_quantity = sigrok::Quantity::POWER;
	else if (signal_name.startsWith("F"))
		sr_quantity = sigrok::Quantity::FREQUENCY;
	else
		assert("Unkown signal in PSU / Load");

	shared_ptr<data::AnalogSignal> signal = make_shared<data::AnalogSignal>(
		sr_channel, data::BaseSignal::AnalogChannel, sr_quantity,
		channel_group_name, aquisition_start_timestamp_);

	// TODO
	if (common_time_data) {
		//signal->set_time_data(common_time_data);
	}
	else {
		//signal->set_time_data(init_time_data());
	}

	add_signal_to_maps(signal, sr_channel, channel_group_name);

	//signals_changed();
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
