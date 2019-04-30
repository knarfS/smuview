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
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

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
#include "src/data/properties/uint64property.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"

using std::bad_alloc;
using std::dynamic_pointer_cast;
using std::find;
using std::lock_guard;
using std::make_pair;
using std::make_shared;
using std::map;
using std::pair;
using std::set;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::unique_ptr;
using std::vector;

namespace sv {
namespace devices {

HardwareDevice::HardwareDevice(
		const shared_ptr<sigrok::Context> sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device) :
	BaseDevice(sr_context, sr_device),
	frame_open_(false),
	frame_begin_(false)
{
	// Set options for different device types
	// TODO: Multiple DeviceTypes per HardwareDevice
	device_type_ = DeviceType::Unknown;
	const auto sr_keys = sr_device->driver()->config_keys();
	for (const auto &sr_key : sr_keys) {
		DeviceType dt = deviceutil::get_device_type(sr_key);
		if (dt == DeviceType::PowerSupply ||
				dt == DeviceType::ElectronicLoad ||
				dt == DeviceType::Oscilloscope ||
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

string HardwareDevice::id() const
{
	string conn_id = sr_device()->connection_id();
	if (conn_id.empty()) {
		// NOTE: sigrok doesn't alway return a connection_id.
		conn_id = std::to_string(device_index_);
	}

	return sr_hardware_device()->driver()->name() + ":" + conn_id;
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

void HardwareDevice::open()
{
	BaseDevice::open();

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

void HardwareDevice::init_configurables()
{
	// Init Configurables from Channel Groups
	for (const auto &sr_cg_pair : sr_device_->channel_groups()) {
		auto sr_cg = sr_cg_pair.second;
		if (sr_cg->config_keys().size() == 0)
			continue;

		auto cg_c = Configurable::create(
			sr_cg, next_configurable_index_++,
			short_name().toStdString(), device_type_);
		configurable_map_.insert(make_pair(sr_cg_pair.first, cg_c));
	}

	/*
	 * Check if the device configurable has any config key of use for us.
	 * We will ignore the common device config keys like "continuous",
	 * "limit_samples" and "limit_time"
	 */
	size_t device_ck_cnt = 0;
	for (const auto &key : sr_device_->config_keys()) {
		if (deviceutil::get_config_key(key) != ConfigKey::Unknown)
			++device_ck_cnt;
	}
	if (device_ck_cnt == 0)
		return;

	// Init Configurable from Device
	auto d_c = Configurable::create(
		sr_device_, next_configurable_index_++,
		short_name().toStdString(), device_type_);
	configurable_map_.insert(make_pair("", d_c));

	// Sample rate for interleaved samples
	if (d_c->has_get_config(ConfigKey::Samplerate)) {
		samplerate_prop_ = static_pointer_cast<data::properties::UInt64Property>(
			d_c->get_property(ConfigKey::Samplerate));
		cur_samplerate_ = samplerate_prop_->uint64_value();
	}
}

void HardwareDevice::init_channels()
{
	map<string, shared_ptr<sigrok::ChannelGroup>> sr_channel_groups =
		sr_device_->channel_groups();

	// Init Channels from Sigrok Channel Groups
	if (sr_channel_groups.size() > 0) {
		for (const auto &sr_cg_pair : sr_channel_groups) {
			shared_ptr<sigrok::ChannelGroup> sr_cg = sr_cg_pair.second;
			for (const auto &sr_channel : sr_cg->channels()) {
				add_sr_channel(sr_channel, sr_cg->name());
			}
		}
	}

	// Init Channels that are not in a channel group
	vector<shared_ptr<sigrok::Channel>> sr_channels = sr_device_->channels();
	for (const auto &sr_channel : sr_channels) {
		if (sr_channel_map_.count(sr_channel) > 0)
			continue;
		add_sr_channel(sr_channel, "");
	}
}

void HardwareDevice::feed_in_header()
{
}

void HardwareDevice::feed_in_trigger()
{
}

void HardwareDevice::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	/*
	 * TODO: The meta packet is missing the information, to which
	 * channel group / configurable the config key belongs.
	 *
	 * Workaround: First try the device configurable (channel group ""),
	 * then try the other configurables.
	 */
	const auto c = configurable_map_[""];
	if (c && c->feed_in_meta(sr_meta))
		return;

	for (const auto &c_pair : configurable_map_) {
		if (c_pair.first == "")
			continue;

		if (c_pair.second && c_pair.second->feed_in_meta(sr_meta))
			return;
	}
}

void HardwareDevice::feed_in_frame_begin()
{
	// TODO: use std::chrono / std::time
	frame_start_timestamp_ = QDateTime::currentMSecsSinceEpoch() / (double)1000;
	frame_open_ = true;
	frame_begin_ = true;
}

void HardwareDevice::feed_in_frame_end()
{
	frame_open_ = false;
	frame_begin_ = false;
}

void HardwareDevice::feed_in_logic(shared_ptr<sigrok::Logic> sr_logic)
{
	(void)sr_logic;
}

void HardwareDevice::feed_in_analog(shared_ptr<sigrok::Analog> sr_analog)
{
	size_t num_samples = sr_analog->num_samples();
	if (num_samples == 0)
		return;

	lock_guard<recursive_mutex> lock(data_mutex_);

	if (cur_samplerate_ == 0 && samplerate_prop_ != nullptr) {
		// TODO: Don't get a config key while in acquisition -> deadlock!
		//cur_samplerate_ = samplerate_prop_->uint64_value();
	}

	const vector<shared_ptr<sigrok::Channel>> sr_channels = sr_analog->channels();

	unique_ptr<float[]> data(new float[num_samples * sr_channels.size()]);
	sr_analog->get_data_as_float(data.get());
	float *channel_data = data.get();

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

		// TODO: use std::chrono / std::time
		double timestamp;
		if (frame_open_)
			timestamp = frame_start_timestamp_;
		else
			timestamp = QDateTime::currentMSecsSinceEpoch() / (double)1000;

		if (device_type_ == DeviceType::Oscilloscope && frame_begin_) {
			channel->start_new_frame(frame_start_timestamp_);
			frame_begin_ = false;
		}

		channel->push_interleaved_samples(channel_data++, num_samples,
			sr_channels.size(), timestamp, cur_samplerate_, sr_analog);
	}
}

} // namespace devices
} // namespace sv
