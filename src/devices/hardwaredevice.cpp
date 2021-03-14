/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2015 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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
#include <thread>
#include <utility>
#include <vector>

#include <glibmm.h>
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

using std::dynamic_pointer_cast;
using std::lock_guard;
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
	BaseDevice(sr_context, sr_device),
	frame_began_(false),
	frame_start_timestamp_(.0),
	cur_samplerate_(0),
	cur_sample_interval_(0)
{
	// Set options for different device types
	// TODO: Multiple DeviceTypes per HardwareDevice
	// TODO: Use deviceutil::is_supported_driver() instead
	type_ = DeviceType::Unknown;
	const auto sr_keys = sr_device->driver()->config_keys();
	for (const auto &sr_key : sr_keys) {
		DeviceType dt = deviceutil::get_device_type(sr_key);
		if (dt == DeviceType::PowerSupply ||
				dt == DeviceType::ElectronicLoad ||
				dt == DeviceType::Oscilloscope ||
				//dt == DeviceType::DemoDev ||
				dt == DeviceType::Multimeter ||
				dt == DeviceType::SoundLevelMeter ||
				dt == DeviceType::Thermometer ||
				dt == DeviceType::Hygrometer ||
				dt == DeviceType::Energymeter ||
				dt == DeviceType::LcrMeter ||
				dt == DeviceType::Scale ||
				dt == DeviceType::SignalGenerator ||
				dt == DeviceType::Powermeter ||
				dt == DeviceType::Multiplexer) {
			type_ = dt;
			break;
		}
	}
	if (type_ == DeviceType::Unknown)
		assert("Unknown device");
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
	/*
	if (sr_hardware_device()->driver()->name() == "demo") {
		sr_device_->config_set(
			sigrok::ConfigKey::SAMPLERATE,
			Glib::Variant<uint64_t>::create(5));
	}
	*/
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

	// Sample rate for multi value samples
	// TODO: This is when rate/interval is changed in sv?? TEST!
	if (d_c->has_get_config(ConfigKey::Samplerate)) {
		auto samplerate_prop =
			static_pointer_cast<data::properties::UInt64Property>(
				d_c->get_property(ConfigKey::Samplerate));
		cur_samplerate_ = samplerate_prop->uint64_value();
		connect(samplerate_prop.get(),
			&data::properties::UInt64Property::change_value,
			this, [this, &samplerate_prop]() {
				cur_samplerate_ = samplerate_prop->uint64_value();
			});
	}
	if (d_c->has_get_config(ConfigKey::SampleInterval)) {
		auto sampleInterval_prop =
			static_pointer_cast<data::properties::UInt64Property>(
				d_c->get_property(ConfigKey::SampleInterval));
		cur_sample_interval_ = sampleInterval_prop->uint64_value();
		connect(sampleInterval_prop.get(),
			&data::properties::UInt64Property::change_value,
			this, [this, &sampleInterval_prop]() {
				cur_samplerate_ = sampleInterval_prop->uint64_value();
			});
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
				auto channel = add_sr_channel(sr_channel, sr_cg->name(),
					channels::ChannelType::AnalogChannel);
			}
		}
	}

	// Init Channels that are not in a channel group
	vector<shared_ptr<sigrok::Channel>> sr_channels = sr_device_->channels();
	for (const auto &sr_channel : sr_channels) {
		if (sr_channel_map_.count(sr_channel) > 0)
			continue;
		auto channel = add_sr_channel(sr_channel, "",
			channels::ChannelType::AnalogChannel);
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
	 * For some devices there are no config keys (get/set/list) for Samplerate
	 * and SampleInterval, but only meta packets. Handel them here and set a
	 * current rate/interval directly to the device. This will be needed for
	 * analog packets mit multiple samples per packet to calculate the time
	 * stride. But also forward them to the configurable.
	 */
	for (const auto &entry : sr_meta->config()) {
		// TODO: !configurable_map_[""]->has_get_config(ConfigKey::Samplerate)
		if (entry.first == sigrok::ConfigKey::SAMPLERATE) {
			Glib::VariantBase gvar = entry.second;
			cur_samplerate_ = g_variant_get_uint64(gvar.gobj());
			cur_sample_interval_ = 0;
		}
		// TODO: !configurable_map_[""]->has_get_config(ConfigKey::SampleInterval)
		else if (entry.first == sigrok::ConfigKey::SAMPLE_INTERVAL) {
			Glib::VariantBase gvar = entry.second;
			cur_sample_interval_ = g_variant_get_uint64(gvar.gobj());
			cur_samplerate_ = 0;
		}
	}

	/*
	 * TODO: The meta packet is missing the information, to which
	 * channel group / configurable the config key belongs to.
	 *
	 * Workaround: First try the device configurable (channel group ""),
	 * then try the other configurables.
	 */

	const auto c = configurable_map_[""];
	if (c && c->feed_in_meta(sr_meta))
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
	frame_start_timestamp_ = QDateTime::currentMSecsSinceEpoch() / (double)1000;
	frame_began_ = true;
}

void HardwareDevice::feed_in_frame_end()
{
	if (!frame_began_)
		return;

	for (const auto &ch_name_pair : channel_map_) {
		 auto channel = dynamic_pointer_cast<channels::HardwareChannel>(
			 ch_name_pair.second);
		 if (channel != nullptr)
			 channel->close_frame();
	}

	frame_began_ = false;
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

	// TODO: use std::chrono / std::time
	double timestamp;
	if (frame_began_)
		timestamp = frame_start_timestamp_;
	else
		timestamp = QDateTime::currentMSecsSinceEpoch() / (double)1000;

	lock_guard<recursive_mutex> lock(data_mutex_);

	auto sr_channels = sr_analog->channels();
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

		if (sr_channel_map_.count(sr_channel) == 0)
			assert("Unknown channel");
		auto channel = static_pointer_cast<channels::HardwareChannel>(
			sr_channel_map_[sr_channel]);

		channel->push_interleaved_samples(
			channel_data++, num_samples, sr_channels.size(),
			timestamp, cur_samplerate_, cur_sample_interval_, sr_analog);
	}
}

} // namespace devices
} // namespace sv
