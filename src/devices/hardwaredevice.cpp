/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2015 Joel Holdsworth <joel@airwebreathe.org.uk>
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
#include <glib.h>
#include <thread>

#include <QDateTime>
#include <QDebug>
#include <QString>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "hardwaredevice.hpp"
#include "src/devicemanager.hpp"
#include "src/session.hpp"
#include "src/devices/channel.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/device.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"

using std::bad_alloc;
using std::dynamic_pointer_cast;
using std::find;
using std::lock_guard;
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
		const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device) :
	Device(sr_context, sr_device)
{
	map<string, shared_ptr<sigrok::ChannelGroup>> sr_channel_groups =
		sr_device_->channel_groups();

	// Init Configurables from Channel Groups and Device
	if (sr_channel_groups.size() > 0) {
		for (auto sr_cg_pair : sr_channel_groups) {
			shared_ptr<sigrok::ChannelGroup> sr_cg = sr_cg_pair.second;
			configurables_.push_back(
				make_shared<devices::Configurable>(sr_cg, short_name()));
		}
	}
	else {
		configurables_.push_back(
			make_shared<Configurable>(sr_device_, short_name()));
	}

	// Init Channels from Sigrok Channel Groups
	if (sr_channel_groups.size() > 0) {
		for (auto sr_cg_pair : sr_channel_groups) {
			shared_ptr<sigrok::ChannelGroup> sr_cg = sr_cg_pair.second;
			QString cg_name = QString::fromStdString(sr_cg->name());
			for (auto sr_channel : sr_cg->channels()) {
				init_channel(sr_channel, cg_name);
			}
		}
	}

	// Init Channels that are not in a channel group
	vector<shared_ptr<sigrok::Channel>> sr_channels = sr_device_->channels();
	for (auto sr_channel : sr_channels) {
		if (sr_channel_map_.count(sr_channel) > 0)
			continue;
		init_channel(sr_channel, QString(""));
	}
}

HardwareDevice::~HardwareDevice()
{
	close();
}

HardwareDevice::Type HardwareDevice::type() const
{
	return type_;
}

QString HardwareDevice::name() const
{
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

	if (sr_device_->connection_id().length() > 0) {
		name.append(sep);
		name.append("(");
		name.append(QString::fromStdString(sr_device_->connection_id()));
		name.append(")");
	}

	return name;
}

QString HardwareDevice::short_name() const
{
	QString sep("");
	QString name("");

	if (sr_device_->vendor().length() > 0) {
		name.append(QString::fromStdString(sr_device_->vendor()));
		sep = QString(" ");
	}

	if (sr_device_->model().length() > 0) {
		name.append(sep);
		name.append(QString::fromStdString(sr_device_->model()));
	}

	return name;
}

QString HardwareDevice::full_name() const
{
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

	if (sr_device_->version().length() > 0) {
		name.append(sep);
		name.append(QString::fromStdString(sr_device_->version()));
		sep = QString(" ");
	}

	if (sr_device_->serial_number().length() > 0) {
		name.append(sep);
		name.append(QString::fromStdString(sr_device_->serial_number()));
		sep = QString(" ");
	}

	if (sr_device_->connection_id().length() > 0) {
		name.append(sep);
		name.append("(");
		name.append(QString::fromStdString(sr_device_->connection_id()));
		name.append(")");
	}

	return name;
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

shared_ptr<sigrok::HardwareDevice> HardwareDevice::sr_hardware_device() const
{
	return static_pointer_cast<sigrok::HardwareDevice>(sr_device_);
}

vector<shared_ptr<devices::Configurable>> HardwareDevice::configurables() const
{
	return configurables_;
}

map<QString, shared_ptr<devices::Channel>> HardwareDevice::channel_name_map() const
{
	return channel_name_map_;
}

map<shared_ptr<sigrok::Channel>, shared_ptr<devices::Channel>> HardwareDevice::sr_channel_map() const
{
	return sr_channel_map_;
}

map<QString, vector<shared_ptr<devices::Channel>>> HardwareDevice::channel_group_name_map() const
{
	return channel_group_name_map_;
}

vector<shared_ptr<data::AnalogSignal>> HardwareDevice::all_signals() const
{
	return all_signals_;
}

shared_ptr<devices::Channel> HardwareDevice::init_channel(
	shared_ptr<sigrok::Channel> sr_channel, QString channel_group_name)
{
	shared_ptr<devices::Channel> channel = make_shared<devices::Channel>(
		sr_channel, Channel::ChannelType::AnalogChannel,
		channel_group_name, aquisition_start_timestamp_);

	// map<QString, shared_ptr<devices::Channel>> channel_name_map_;
	channel_name_map_.insert(
		pair<QString, shared_ptr<devices::Channel>>
		(channel->internal_name(), channel));

	// map<shared_ptr<sigrok::Channel>, shared_ptr<devices::Channel>> sr_channel_map_;
	sr_channel_map_.insert(
		pair<shared_ptr<sigrok::Channel>, shared_ptr<devices::Channel>>
		(sr_channel, channel));

	// map<QString, vector<shared_ptr<devices::Channel>>> channel_group_name_map_;
	if (channel_group_name_map_.count(channel_group_name) == 0)
		channel_group_name_map_.insert(
			pair<QString, vector<shared_ptr<devices::Channel>>>
			(channel_group_name, vector<shared_ptr<devices::Channel>>()));
	channel_group_name_map_[channel_group_name].push_back(channel);

	return channel;
}

void HardwareDevice::feed_in_header()
{
}

void HardwareDevice::feed_in_trigger()
{
}

void HardwareDevice::feed_in_frame_begin()
{
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

	const vector<shared_ptr<sigrok::Channel>> sr_channels = sr_analog->channels();
	//const unsigned int channel_count = sr_channels.size();
	//const size_t sample_count = sr_analog->num_samples() / channel_count;

	unique_ptr<float> data(new float[sr_analog->num_samples()]);
	sr_analog->get_data_as_float(data.get());

	float *channel_data = data.get();
	for (auto sr_channel : sr_channels) {
		/*
		qWarning() << "HardwareDevice::feed_in_analog(): HardwareDevice = " <<
			QString::fromStdString(sr_device_->model()) <<
			", Channel.Id = " <<
			QString::fromStdString(sr_channel->name()) <<
			" channel_data = " << *channel_data;
		*/

		if (!sr_channel_map_.count(sr_channel))
			assert("Unknown channel");
		shared_ptr<devices::Channel> channel = sr_channel_map_[sr_channel];

		/*
		 * TODO: Use push_interleaved_samples() as only push function
		actual_signal->analog_data()->push_interleaved_samples(
			channel_data, sample_count,channel_count, sr_analog->unit());
		*/

		if (frame_began_) {
			channel->push_sample(
				channel_data, frame_start_timestamp_,
				sr_analog->mq(), sr_analog->mq_flags(), sr_analog->unit());
		}
		else {
			channel->push_sample(
				channel_data,
				sr_analog->mq(), sr_analog->mq_flags(), sr_analog->unit());
		}
		channel_data++;
	}
}

} // namespace devices
} // namespace sv
