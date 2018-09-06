/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>

#include "virtualdevice.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/deviceutil.hpp"

using std::static_pointer_cast;

namespace sv {
namespace devices {

VirtualDevice::VirtualDevice(
		const shared_ptr<sigrok::Context> &sr_context,
		QString vendor, QString model, QString version) :
	BaseDevice(sr_context, nullptr),
	vendor_(vendor),
	model_(model),
	version_(version),
	channel_index_(0)
{
	auto sr_device = sr_context_->create_user_device(
		vendor_.toStdString(), model_.toStdString(), version_.toStdString());
	sr_device_ = sr_device;
	device_type_ = DeviceType::VirtualDevice;
}

QString VirtualDevice::name() const
{
	return short_name();
}

QString VirtualDevice::short_name() const
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

QString VirtualDevice::full_name() const
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

QString VirtualDevice::display_name(
	const DeviceManager &device_manager) const
{
	(void)device_manager;
	return full_name();
}

/* TODO: merge with Device */
void VirtualDevice::open(function<void (const QString)> error_handler)
{
	(void)error_handler;

	if (device_open_)
		close();

	try {
		sr_device_->open();
	}
	catch (const sigrok::Error &e) {
		// TODO: UserDevices throws SR_ERR_ARG in device.sr_dev_open(), thats ok, bc the device has no driver
		//throw QString(e.what());
	}

	// Add device to session (do this in constructor??)
	sr_session_->add_device(sr_device_);

	device_open_ = true;
}

/* TODO: merge with Device */
void VirtualDevice::close()
{
	if (!device_open_)
		return;

	sr_session_->remove_devices();
	sr_device_->close();
	device_open_ = false;
}

void VirtualDevice::add_channel(shared_ptr<channels::BaseChannel> channel,
	QString channel_group_name)
{
	auto sr_user_device = static_pointer_cast<sigrok::UserDevice>(sr_device_);
	sr_user_device->add_channel(
		channel_index_, sigrok::ChannelType::ANALOG, name().toStdString());
	channel_index_++;
	BaseDevice::add_channel(channel, channel_group_name);
}

void VirtualDevice::init_channels()
{
}

void VirtualDevice::init_configurables()
{
}

void VirtualDevice::feed_in_header()
{
}

void VirtualDevice::feed_in_trigger()
{
}

void VirtualDevice::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	(void)sr_meta;
}

void VirtualDevice::feed_in_frame_begin()
{
}

void VirtualDevice::feed_in_frame_end()
{
}

void VirtualDevice::feed_in_logic(shared_ptr<sigrok::Logic> sr_logic)
{
	(void)sr_logic;
}

void VirtualDevice::feed_in_analog(shared_ptr<sigrok::Analog> sr_analog)
{
	(void)sr_analog;
}


} // namespace devices
} // namespace sv
