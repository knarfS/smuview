/*
 * This file is part of the SmuView project.
 *
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
		string vendor, string model, string version) :
	BaseDevice(sr_context, nullptr),
	vendor_(vendor),
	model_(model),
	version_(version),
	channel_index_(0)
{
	auto sr_device = sr_context_->create_user_device(vendor_, model_, version_);
	sr_device_ = sr_device;
	device_type_ = DeviceType::UserDevice;
}

string UserDevice::id() const
{
	string conn_id = sr_device()->connection_id();
	if (conn_id.empty()) {
		// NOTE: sigrok doesn't alway return a connection_id.
		conn_id = std::to_string(device_index_);
	}

	return "userdevice:" + conn_id;
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

/* TODO: merge with Device */
void UserDevice::open()
{
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
void UserDevice::close()
{
	qWarning() << "UserDevice::close(): Trying to close device " << full_name();

	if (!device_open_)
		return;

	sr_session_->stop();

	/*
	 * NOTE: The device may already be closed from sr_session_->stop()
	 *
	 * sigrok::Session::stop() -> sr_session_stop() -> session_stop_sync() ->
	 * sr_dev_acquisition_stop() -> via devce api dev_acquisition_stop() ->
	 * std_serial_dev_acquisition_stop() -> sr_dev_close()
	 */
	try {
		sr_device_->close();
	}
	catch (...) {}

	if (sr_session_)
		sr_session_->remove_devices();

	device_open_ = false;

	qWarning() << "UserDevice::close(): Device closed " << full_name();
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
