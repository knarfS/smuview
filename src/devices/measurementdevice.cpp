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
	// Set options for different device types
	const auto sr_keys = sr_device->driver()->config_keys();
	if (sr_keys.count(sigrok::ConfigKey::MULTIMETER))
		type_ = HardwareDevice::MULTIMETER;
	else if (sr_keys.count(sigrok::ConfigKey::DEMO_DEV))
		type_ = HardwareDevice::DEMO_DEV;
	else
		assert("Unknown device");
}

MeasurementDevice::~MeasurementDevice()
{
}

/*
shared_ptr<data::BaseSignal> MeasurementDevice::init_signal(
	shared_ptr<sigrok::Channel> sr_channel,
	QString channel_group_name)
{
	if (sr_channel->type()->id() != SR_CHANNEL_ANALOG)
		return nullptr;

	//lock_guard<recursive_mutex> lock(data_mutex_);

	shared_ptr<data::AnalogSignal> signal = make_shared<data::AnalogSignal>(
		sr_channel, data::BaseSignal::AnalogChannel, nullptr,
		channel_group_name, aquisition_start_timestamp_);

	add_signal_to_maps(signal, sr_channel, channel_group_name);

	//signals_changed();

	return signal;
}
*/

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
