/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>

#include "userdevice.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/deviceutil.hpp"

using std::static_pointer_cast;

namespace sv {
namespace devices {

UserDevice::UserDevice(
		const shared_ptr<sigrok::Context> &sr_context,
		const string &vendor, const string &model, const string &version) :
	BaseDevice(sr_context, nullptr),
	vendor_(vendor),
	model_(model),
	version_(version),
	channel_index_(0)
{
	sr_device_ = sr_context_->create_user_device(vendor_, model_, version_);
	type_ = DeviceType::UserDevice;
}

string UserDevice::name() const
{
	return short_name().toStdString();
}

QString UserDevice::full_name() const
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

	return name;
}

QString UserDevice::short_name() const
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

QString UserDevice::display_name(
	const DeviceManager &device_manager) const
{
	(void)device_manager;
	return full_name();
}

void UserDevice::add_channel(shared_ptr<channels::BaseChannel> channel,
	string channel_group_name)
{
	auto sr_user_device = static_pointer_cast<sigrok::UserDevice>(sr_device_);
	sr_user_device->add_channel(
		channel_index_, sigrok::ChannelType::ANALOG, name());
	channel_index_++;
	BaseDevice::add_channel(channel, channel_group_name);
}

void UserDevice::init_configurables()
{
}

void UserDevice::init_channels()
{
}

void UserDevice::init_acquisition()
{
}

void UserDevice::feed_in_header()
{
}

void UserDevice::feed_in_trigger()
{
}

void UserDevice::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	(void)sr_meta;
}

void UserDevice::feed_in_frame_begin()
{
}

void UserDevice::feed_in_frame_end()
{
}

void UserDevice::feed_in_logic(shared_ptr<sigrok::Logic> sr_logic)
{
	(void)sr_logic;
}

void UserDevice::feed_in_analog(shared_ptr<sigrok::Analog> sr_analog)
{
	(void)sr_analog;
}


} // namespace devices
} // namespace sv
