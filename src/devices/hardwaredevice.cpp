/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2015 Joel Holdsworth <joel@airwebreathe.org.uk>
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
#include <functional>
#include <thread>
#include <utility>

#include <glib.h>

#include <QDateTime>
#include <QDebug>
#include <QString>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "hardwaredevice.hpp"
#include "src/devicemanager.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"

using std::bad_alloc;
using std::dynamic_pointer_cast;
using std::find;
using std::function;
using std::lock_guard;
using std::make_pair;
using std::make_shared;
using std::map;
using std::pair;
using std::set;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::vector;
using std::unique_ptr;

namespace sv {
namespace devices {

HardwareDevice::HardwareDevice(
		const shared_ptr<sigrok::Context> sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device) :
	BaseDevice(sr_context, sr_device)
{
	// Set options for different device types
	// TODO: Multiple DeviceTypes per HardwareDevice
	device_type_ = DeviceType::Unknown;
	const auto sr_keys = sr_device->driver()->config_keys();
	for (const auto &sr_key : sr_keys) {
		DeviceType dt = deviceutil::get_device_type(sr_key);
		if (dt == DeviceType::PowerSupply ||
				dt == DeviceType::ElectronicLoad ||
				dt == DeviceType::DemoDev ||
				dt == DeviceType::Multimeter ||
				dt == DeviceType::SoundLevelMeter ||
				dt == DeviceType::Thermometer ||
				dt == DeviceType::Hygrometer ||
				dt == DeviceType::Energymeter ||
				dt == DeviceType::LcrMeter ||
				dt == DeviceType::Scale ||
				dt == DeviceType::Powermeter) {
			device_type_ = dt;
			break;
		}
	}
	if (device_type_ == DeviceType::Unknown)
		assert("Unknown device");
}

void HardwareDevice::init()
{
	// Init Configurables from Channel Groups
	map<string, shared_ptr<sigrok::ChannelGroup>> sr_channel_groups =
		sr_device_->channel_groups();
	if (sr_channel_groups.size() > 0) {
		for (const auto &sr_cg_pair : sr_channel_groups) {
			auto sr_cg = sr_cg_pair.second;
			auto cg_c = Configurable::create(sr_cg, short_name(), device_type_);
			configurables_.push_back(cg_c);
		}
	}

	// Init Configurable from Device
	auto d_c = Configurable::create(sr_device_, short_name(), device_type_);
	configurables_.push_back(d_c);
}

string HardwareDevice::id() const
{
	// TODO: Add index, when more then one of the same device exists.
	return sr_hardware_device()->driver()->name();
}

QString HardwareDevice::display_name(
	const DeviceManager &device_manager) const
{
	const auto hw_dev = sr_hardware_device();

	// If we can find another device with the same model/vendor then
	// we have at least two such devices and need to distinguish them.
	const auto &devices = device_manager.devices();
	const bool multiple_dev = hw_dev && any_of(
		devices.begin(), devices.end(),
		[&](shared_ptr<devices::HardwareDevice> dev) {
			return dev->sr_hardware_device()->vendor() ==
					hw_dev->vendor() &&
				dev->sr_hardware_device()->model() ==
					hw_dev->model() &&
				dev->sr_device_ != sr_device_;
		});

	QString sep("");
	QString name("");

	if (sr_device_->vendor().length() > 0) {
		name.append(QString::fromStdString(sr_device_->vendor()));
		sep = QString(" ");
	}

	if (sr_device_->model().length() > 0) {
		name.append(sep);
		name.append(QString::fromStdString(sr_device_->model()));
		sep = QString(" ");
	}

	if (multiple_dev) {
		if (sr_device_->model().length() > 0) {
			name.append(sep);
			name.append(QString::fromStdString(sr_device_->version()));
			sep = QString(" ");
		}

		if (sr_device_->model().length() > 0) {
			name.append(sep);
			name.append(QString::fromStdString(sr_device_->serial_number()));
			sep = QString(" ");
		}

		if ((sr_device_->serial_number().length() == 0) &&
				(sr_device_->connection_id().length() > 0)) {
			name.append(sep);
			name.append("(");
			name.append(QString::fromStdString(sr_device_->connection_id()));
			name.append(")");
		}
	}

	return name;
}

void HardwareDevice::open(function<void (const QString)> error_handler)
{
	BaseDevice::open(error_handler);

	// Special handling for device "demo": Set a initial moderate samplerate of
	// 5 samples per second, to slow down the analog channels.
	if (sr_hardware_device()->driver()->name() == "demo") {
		sr_device_->config_set(
			sigrok::ConfigKey::SAMPLERATE,
			Glib::Variant<uint64_t>::create(5));
	}
}

shared_ptr<sigrok::HardwareDevice> HardwareDevice::sr_hardware_device() const
{
	return static_pointer_cast<sigrok::HardwareDevice>(sr_device_);
}

vector<shared_ptr<devices::Configurable>> HardwareDevice::configurables() const
{
	return configurables_;
}

map<shared_ptr<sigrok::Channel>, shared_ptr<channels::BaseChannel>>
	HardwareDevice::sr_channel_map() const
{
	return sr_channel_map_;
}

void HardwareDevice::init_channels()
{
	map<string, shared_ptr<sigrok::ChannelGroup>> sr_channel_groups =
		sr_device_->channel_groups();

	// Init Channels from Sigrok Channel Groups
	if (sr_channel_groups.size() > 0) {
		for (const auto &sr_cg_pair : sr_channel_groups) {
			shared_ptr<sigrok::ChannelGroup> sr_cg = sr_cg_pair.second;
			QString cg_name = QString::fromStdString(sr_cg->name());
			for (const auto &sr_channel : sr_cg->channels()) {
				init_channel(sr_channel, cg_name);
			}
		}
	}

	// Init Channels that are not in a channel group
	vector<shared_ptr<sigrok::Channel>> sr_channels = sr_device_->channels();
	for (const auto &sr_channel : sr_channels) {
		if (sr_channel_map_.count(sr_channel) > 0)
			continue;
		init_channel(sr_channel, QString(""));
	}
}

shared_ptr<channels::BaseChannel> HardwareDevice::init_channel(
	shared_ptr<sigrok::Channel> sr_channel, QString channel_group_name)
{
	shared_ptr<channels::HardwareChannel> channel =
		make_shared<channels::HardwareChannel>(sr_channel,
			shared_from_this(), channel_group_name, aquisition_start_timestamp_);

	add_channel(channel, channel_group_name);

	// map<shared_ptr<sigrok::Channel>, shared_ptr<channels::BaseChannel>> sr_channel_map_;
	sr_channel_map_.insert(make_pair(sr_channel, channel));

	return channel;
}

void HardwareDevice::feed_in_header()
{
}

void HardwareDevice::feed_in_trigger()
{
}

void HardwareDevice::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	// TODO: The meta packet is missing the information, to which
	// channel group the config key belongs.
	if (configurables_.size() > 0)
		configurables_.front()->feed_in_meta(sr_meta);
}

void HardwareDevice::feed_in_frame_begin()
{
	// TODO: use std::chrono / std::time
	frame_start_timestamp_ = QDateTime::currentMSecsSinceEpoch() / (double)1000;
	frame_began_ = true;
}

void HardwareDevice::feed_in_frame_end()
{
	frame_began_ = false;
}

void HardwareDevice::feed_in_logic(shared_ptr<sigrok::Logic> sr_logic)
{
	(void)sr_logic;
}

void HardwareDevice::feed_in_analog(shared_ptr<sigrok::Analog> sr_analog)
{
	lock_guard<recursive_mutex> lock(data_mutex_);

	unique_ptr<float> data(new float[sr_analog->num_samples()]);
	sr_analog->get_data_as_float(data.get());
	float *channel_data = data.get();

	const vector<shared_ptr<sigrok::Channel>> sr_channels = sr_analog->channels();
	for (const auto &sr_channel : sr_channels) {
		/*
		qWarning() << "HardwareDevice::feed_in_analog(): HardwareDevice = " <<
			QString::fromStdString(sr_device_->model()) <<
			", Channel.Id = " <<
			QString::fromStdString(sr_channel->name()) <<
			" channel_data = " << *channel_data;
		*/

		if (!sr_channel_map_.count(sr_channel))
			assert("Unknown channel");
		auto channel = static_pointer_cast<channels::HardwareChannel>(
			sr_channel_map_[sr_channel]);

		if (frame_began_) {
			channel->push_sample_sr_analog(
				channel_data, frame_start_timestamp_, sr_analog);
		}
		else {
			// TODO: use std::chrono / std::time
			double timestamp =
				QDateTime::currentMSecsSinceEpoch() / (double)1000;
			channel->push_sample_sr_analog(channel_data, timestamp, sr_analog);
		}
		channel_data++;
	}
}

} // namespace devices
} // namespace sv
