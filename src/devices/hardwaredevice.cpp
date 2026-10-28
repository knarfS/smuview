/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2015 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <glib.h>

#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QStringList>

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

using std::make_pair;
using std::map;
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
	BaseDevice(sr_context, sr_device)
{
	// TODO: Multiple DeviceTypes per HardwareDevice

	// Check for the demo device first, as the demo device can have multiple
	// types like oscilloscope. We have to make sure, that this HardwareDevice
	// is of the type DemoDevice, with the highest priority.
	const auto &device_types = deviceutil::get_device_types(sr_device->driver());
	if (std::any_of(device_types.begin(), device_types.end(),
			deviceutil::is_demo_device)) {
		type_ = DeviceType::DemoDev;
		return;
	}

	// Now check all other device types
	const auto &it = std::find_if(device_types.begin(), device_types.end(),
		deviceutil::is_supported_device);

	if (it != device_types.end())
		type_ = *it;
	else
		throw std::runtime_error("Unknown device type");
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
			return dev->sr_hardware_device()->vendor() == hw_dev->vendor() &&
				dev->sr_hardware_device()->model() == hw_dev->model() &&
				dev->sr_device_ != sr_device_;
		});

	QStringList parts;

	if (!sr_device_->vendor().empty())
		parts << QString::fromStdString(sr_device_->vendor());

	if (!sr_device_->model().empty())
		parts << QString::fromStdString(sr_device_->model());

	if (multiple_dev) {
		if (!sr_device_->version().empty())
			parts << QString::fromStdString(sr_device_->version());

		if (!sr_device_->serial_number().empty())
			parts << "[S/N: " +
				QString::fromStdString(sr_device_->serial_number()) + "]";
		else if (!sr_device_->connection_id().empty())
			parts << "(" +
				QString::fromStdString(sr_device_->connection_id()) + ")";
	}

	return parts.join(" ");
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
		if (sr_cg->config_keys().empty())
			continue;

		auto cg_c = Configurable::create(
			sr_cg, next_configurable_index_++,
			short_name().toStdString(), type_, settings_id());
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
		short_name().toStdString(), type_, settings_id());
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
	if (!sr_channel_groups.empty()) {
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
	 * channel group / configurable the config key belongs to.
	 *
	 * Workaround: First try the device configurable (channel group ""),
	 * then try the other configurables.
	 */

	const auto it = configurable_map_.find("");
	if (it != configurable_map_.end()
			&& it->second && it->second->feed_in_meta(sr_meta))
		return;

	for (const auto &c_pair : configurable_map_) {
		if (c_pair.first.empty())
			continue;
		if (c_pair.second && c_pair.second->feed_in_meta(sr_meta))
			return;
	}
}

void HardwareDevice::feed_in_frame_begin()
{
	// TODO: use std::chrono / std::time
	frame_start_timestamp_ =
		static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;
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
	size_t sample_count = sr_analog->num_samples();
	if (sample_count == 0)
		return;

	/*
	 * NOTE: Sometimes the mq is not set (e.g. for the demo driver in
	 *       sigrok 6.0.0) and mq() just throws an exception, without a
	 *       possibility to check if mq is set or not.
	 */
	data::Quantity quantity;
	try {
		quantity = data::datautil::get_quantity(sr_analog->mq());
	}
	catch (const sigrok::Error &e) {
		quantity = data::Quantity::Unknown;
	}
	set<data::QuantityFlag> quantity_flags =
		data::datautil::get_quantity_flags(sr_analog->mq_flags());
	data::Unit unit = data::datautil::get_unit(sr_analog->unit());

	uint64_t samplerate = samplerate_prop_ != nullptr
		? samplerate_prop_->uint64_value() : 0;

	// TODO: use std::chrono / std::time
	double timestamp = frame_began_
		? frame_start_timestamp_
		: static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;

	const auto sr_channels = sr_analog->channels();
	size_t channel_count = sr_channels.size();

	unique_ptr<float[]> data(new float[sample_count * channel_count]);
	sr_analog->get_data_as_float(data.get());

	for (size_t ch_idx = 0; ch_idx < channel_count; ch_idx++) {
		const auto it = sr_channel_map_.find(sr_channels.at(ch_idx));
		if (it == sr_channel_map_.end() || !it->second) {
			qWarning() << "HardwareDevice::feed_in_analog(): "
				<< "Could not find sr_channel "
				<< QString::fromStdString(sr_channels.at(ch_idx)->name());
			continue;
		}

		auto channel =
			static_pointer_cast<channels::HardwareChannel>(it->second);

		vector<double> samples = deviceutil::deinterleave_sigrok_data(
			data.get(), sample_count, ch_idx, channel_count);

		channel->push_samples(samples, timestamp, samplerate,
			quantity, quantity_flags, unit, sr_analog->digits());
	}
}

} // namespace devices
} // namespace sv
