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

	// Init signals. We get all channels from the device, also all cg signals
	vector<shared_ptr<sigrok::Channel>> sr_channels = sr_device_->channels();
	for (auto sr_channel : sr_channels) {
		// TODO: sr_channel is not necessarily a signal (see Digi35)....
		init_signal(sr_channel, common_time_data);
	}
}

SourceSinkDevice::~SourceSinkDevice()
{
	close();
}

void SourceSinkDevice::init_signal(
	shared_ptr<sigrok::Channel> sr_channel,
	shared_ptr<vector<double>> common_time_data)
{
	//lock_guard<recursive_mutex> lock(data_mutex_);

	if (sr_channel->type()->id() != SR_CHANNEL_ANALOG)
		return;

	shared_ptr<data::AnalogSignal> signal;
	QString signal_name = QString::fromStdString(sr_channel->name());

	if (signal_name.startsWith("V")) {
		signal = make_shared<data::AnalogSignal>(sr_channel,
			data::BaseSignal::AnalogChannel, sigrok::Quantity::VOLTAGE,
			aquisition_start_timestamp_);
	}
	else if (signal_name.startsWith("I")) {
		signal = make_shared<data::AnalogSignal>(sr_channel,
			data::BaseSignal::AnalogChannel, sigrok::Quantity::CURRENT,
			aquisition_start_timestamp_);
	}
	else if (signal_name.startsWith("P")) {
		signal = make_shared<data::AnalogSignal>(sr_channel,
			data::BaseSignal::AnalogChannel, sigrok::Quantity::POWER,
			aquisition_start_timestamp_);
	}
	else if (signal_name.startsWith("F")) {
		signal = make_shared<data::AnalogSignal>(sr_channel,
			data::BaseSignal::AnalogChannel, sigrok::Quantity::FREQUENCY,
			aquisition_start_timestamp_);
	}

	if (!signal)
		return;

	// TODO
	if (common_time_data) {
		//signal->set_time_data(common_time_data);
	}
	else {
		//signal->set_time_data(init_time_data());
	}

	all_signals_.push_back(signal);
	signal_name_map_.insert(
		pair<QString, shared_ptr<data::BaseSignal>>
			(signal->internal_name(), signal));
	sr_channel_signal_map_.insert(
		pair<shared_ptr<sigrok::Channel>, shared_ptr<data::BaseSignal>>
			(sr_channel, signal));

	if (signal->internal_name().startsWith("V") && !voltage_signal_)
		voltage_signal_ = signal;
	else if (signal->internal_name().startsWith("I") && !current_signal_)
		current_signal_ = signal;

	//signals_changed();
}

shared_ptr<data::AnalogSignal> SourceSinkDevice::voltage_signal() const
{
	return voltage_signal_;
}

shared_ptr<data::AnalogSignal> SourceSinkDevice::current_signal() const
{
	return current_signal_;
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
