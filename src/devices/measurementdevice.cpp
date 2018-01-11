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

#include "measurementdevice.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace devices {

MeasurementDevice::MeasurementDevice(
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
	if (sr_keys.count(sigrok::ConfigKey::MULTIMETER)) {
		type_ = HardwareDevice::MULTIMETER;
		common_time_data = nullptr;
	}
	else if (sr_keys.count(sigrok::ConfigKey::DEMO_DEV)) {
		type_ = HardwareDevice::DEMO_DEV;
		common_time_data = nullptr;
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

MeasurementDevice::~MeasurementDevice()
{
}

void MeasurementDevice::init_signal(
	shared_ptr<sigrok::Channel> sr_channel,
	shared_ptr<vector<double>> common_time_data)
{
	//lock_guard<recursive_mutex> lock(data_mutex_);

	if (sr_channel->type()->id() != SR_CHANNEL_ANALOG)
		return;

	shared_ptr<data::AnalogSignal> signal = make_shared<data::AnalogSignal>(
		sr_channel, data::BaseSignal::AnalogChannel, sigrok::Quantity::VOLTAGE,
		aquisition_start_timestamp_);

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

	measurement_signal_ = signal;

	//signals_changed();
}

shared_ptr<data::AnalogSignal> MeasurementDevice::measurement_signal() const
{
	return measurement_signal_;
}

void MeasurementDevice::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	// TODO: Move to devices::Configurable and implement device specific signals

	// TODO: The meta packet is missing the information, to which
	// channel group the config key belongs.
	shared_ptr<devices::Configurable> configurable;
	if (configurables_.size() > 0)
		configurable = configurables_.at(0);

	for (auto entry : sr_meta->config()) {
		switch (entry.first->id()) {
		default:
			// Unknown metadata is not an error.
			break;
		}
	}
}

} // namespace devices
} // namespace sv
