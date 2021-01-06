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

#include <cassert>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include <glib.h>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>
#include <QString>
#include <QUuid>

#include "basedevice.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/channels/userchannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"

#define USER_CHANNEL_START_INDEX 1000
#define CONFIGURABLE_START_INDEX 5000

using std::bad_alloc;
using std::dynamic_pointer_cast;
using std::lock_guard;
using std::make_pair;
using std::make_shared;
using std::map;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {
namespace devices {

unsigned int BaseDevice::device_counter = 0;

BaseDevice::BaseDevice(const shared_ptr<sigrok::Context> sr_context,
		shared_ptr<sigrok::Device> sr_device) :
	sr_context_(sr_context),
	sr_device_(sr_device),
	is_open_(false),
	next_channel_index_(USER_CHANNEL_START_INDEX),
	next_configurable_index_(CONFIGURABLE_START_INDEX),
	frame_began_(false)
{
	// Set up a sigrok session per smuvierw device
	sr_session_ = sv::Session::sr_context->create_session();

	// Every device gets its own unique index
	index_ = BaseDevice::device_counter++;

	/*
	 * NOTE: Get the start timestamp from the session.
	 *       This way, combining signals from different devices (export as
	 *       CSV, XY-Plots) can be displayed with relative timestamps.
	 */
	aquisition_start_timestamp_ = sv::Session::session_start_timestamp;
}

BaseDevice::~BaseDevice()
{
	// TODO: Not called!! Maybe cyclic refernece?
	qWarning() << "BaseDevice::~BaseDevice(): " << BaseDevice::full_name();
	if (sr_session_)
		BaseDevice::close();
}

shared_ptr<sigrok::Device> BaseDevice::sr_device() const
{
	return sr_device_;
}

DeviceType BaseDevice::type() const
{
	return type_;
}

string BaseDevice::id() const
{
	string vendor = SettingsManager::format_key(sr_device_->vendor());
	string model = SettingsManager::format_key(sr_device_->model());
	string id = vendor + ":" + model;

	if (!sr_device_->serial_number().empty()) {
		id +=  ":" + SettingsManager::format_key(sr_device_->serial_number());
	}
	else if (!sr_device_->connection_id().empty()) {
		id += ":" + SettingsManager::format_key(sr_device_->connection_id());
	}
	else if (type_ == DeviceType::DemoDev) {
		// Create a random id for the demo device, to make it available and
		// identifiable in the device tree or via the python API.
		id += ":" + util::format_uuid(QUuid::createUuid());
	}

	return id;
}

QString BaseDevice::settings_id() const
{
	if (type_ == DeviceType::DemoDev) {
		string vendor = SettingsManager::format_key(sr_device_->vendor());
		string model = SettingsManager::format_key(sr_device_->model());
		return QString::fromStdString(vendor + ":" + model);
	}
	return QString::fromStdString(this->id());
}

void BaseDevice::open()
{
	if (is_open_)
		close();

	try {
		sr_device_->open();
	}
	catch (const sigrok::Error &e) {
		// TODO: UserDevices throws SR_ERR_ARG in device.sr_dev_open(). That's
		//       ok, b/c the device has no driver.
		//throw QString(e.what());
	}

	// Add device to session (do this in constructor??)
	sr_session_->add_device(sr_device_);

	// Init all configurables
	this->init_configurables();
	// Init all channels
	this->init_channels();
	// Init aquisition
	this->init_acquisition();

	is_open_ = true;
}

void BaseDevice::close()
{
	qWarning() << "BaseDevice::close(): Trying to close device " <<
		BaseDevice::full_name();

	if (!is_open_)
		return;

	sr_session_->stop();

	// Check that sampling stopped
	if (aquisition_thread_.joinable())
		aquisition_thread_.join();
	sr_session_->remove_datafeed_callbacks();
	aquisition_state_ = AquisitionState::Stopped;

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
	catch (const sigrok::Error &e) {
		// TODO: UserDevices throws SR_ERR_ARG in device.sr_dev_close(). That's
		//       ok, b/c the device has no driver.
		//throw QString(e.what());
	}

	if (sr_session_)
		sr_session_->remove_devices();

	is_open_ = false;

	qWarning() << "BaseDevice::close(): Device closed " <<
		BaseDevice::full_name();
}

void BaseDevice::start_aquisition()
{
	aquisition_state_ = AquisitionState::Running;
}

void BaseDevice::pause_aquisition()
{
	aquisition_state_ = AquisitionState::Paused;
}

string BaseDevice::name() const
{
	string sep;
	string name;

	if (sr_device_->vendor().length() > 0) {
		name += sr_device_->vendor();
		sep = " ";
	}

	if (sr_device_->model().length() > 0) {
		name += sep + sr_device_->model();
		sep = " ";
	}

	if (sr_device_->connection_id().length() > 0) {
		name += sep + "(" + sr_device_->connection_id() + ")";
	}

	return name;
}

QString BaseDevice::full_name() const
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

QString BaseDevice::short_name() const
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

map<string, shared_ptr<devices::Configurable>>
	BaseDevice::configurable_map() const
{
	return configurable_map_;
}

map<string, shared_ptr<channels::BaseChannel>> BaseDevice::channel_map() const
{
	return channel_map_;
}

map<string, vector<shared_ptr<channels::BaseChannel>>>
	BaseDevice::channel_group_map() const
{
	return channel_group_map_;
}

map<shared_ptr<sigrok::Channel>, shared_ptr<channels::BaseChannel>>
	BaseDevice::sr_channel_map() const
{
	return sr_channel_map_;
}

vector<shared_ptr<data::BaseSignal>> BaseDevice::signals() const
{
	vector<shared_ptr<sv::data::BaseSignal>> signals;
	for (const auto &ch_pair : channel_map_) {
		for (const auto &signal_pair : ch_pair.second->signal_map()) {
			for (const auto &signal : signal_pair.second) {
				signals.push_back(signal);
			}
		}
	}
	return signals;
}

unsigned int BaseDevice::next_channel_index()
{
	return next_channel_index_++;
}


void BaseDevice::add_channel(shared_ptr<channels::BaseChannel> channel,
	const string &channel_group_name)
{
	// Check if channel already exists. Channel names are unique per device.
	if (channel_map_.count(channel->name()) == 0) {
		connect(this, SIGNAL(aquisition_start_timestamp_changed(double)),
			channel.get(), SLOT(on_aquisition_start_timestamp_changed(double)));

		// map<QString, shared_ptr<channels::BaseChannel>> channel_name_map_;
		channel_map_.insert(make_pair(channel->name(), channel));
	}

	// map<QString, vector<shared_ptr<channels::BaseChannel>>> channel_group_name_map_;
	if (channel_group_map_.count(channel_group_name) == 0) {
		channel_group_map_.insert(make_pair(
			channel_group_name, vector<shared_ptr<channels::BaseChannel>>()));
	}
	channel_group_map_[channel_group_name].push_back(channel);

	if (channel->channel_group_names().count(channel_group_name) == 0) {
		channel->add_channel_group_name(channel_group_name);
	}

	Q_EMIT channel_added(channel);
}

shared_ptr<channels::BaseChannel> BaseDevice::add_sr_channel(
	shared_ptr<sigrok::Channel> sr_channel, const string &channel_group_name)
{
	// Check if channel already exists.
	// NOTE: Channel names are unique per device.
	shared_ptr<channels::BaseChannel> channel;
	if (channel_map_.count(sr_channel->name()) > 0) {
		channel = channel_map()[sr_channel->name()];
	}
	else {
		set<string> chg_names { channel_group_name };
		channel = make_shared<channels::HardwareChannel>(sr_channel,
			shared_from_this(), chg_names, aquisition_start_timestamp_);

		// map<shared_ptr<sigrok::Channel>, shared_ptr<channels::BaseChannel>> sr_channel_map_;
		sr_channel_map_.insert(make_pair(sr_channel, channel));
	}

	add_channel(channel, channel_group_name);

	return channel;
}

void BaseDevice::add_math_channel(
	shared_ptr<channels::MathChannel> math_channel,
	const string &channel_group_name)
{
	add_channel(math_channel, channel_group_name);

	/*
	 * TODO: Remove shared_from_this() / (channel pointer in signal), so that
	 *       "add_signal()" can be called from MathChannel ctor.
	 */
	math_channel->add_signal(
		math_channel->quantity(),
		math_channel->quantity_flags(),
		math_channel->unit());
}

shared_ptr<channels::UserChannel> BaseDevice::add_user_channel(
	const string &channel_name, const string &channel_group_name)
{
	shared_ptr<channels::UserChannel> channel =
		make_shared<channels::UserChannel>(
			channel_name, set<string> { channel_group_name },
			shared_from_this(), aquisition_start_timestamp_);
	add_channel(channel, channel_group_name);

	return channel;
}

void BaseDevice::init_acquisition()
{
	sr_session_->add_datafeed_callback([=]
		(shared_ptr<sigrok::Device> sr_device, shared_ptr<sigrok::Packet> sr_packet) {
			data_feed_in(sr_device, sr_packet);
		});
	aquisition_thread_ = std::thread(
		&BaseDevice::aquisition_thread_proc, this);
	aquisition_state_ = AquisitionState::Running;
}

void BaseDevice::data_feed_in(shared_ptr<sigrok::Device> sr_device,
	shared_ptr<sigrok::Packet> sr_packet)
{
	/*
	qWarning() << "data_feed_in(): sr_packet->type()->id() = "
		<< sr_packet->type()->id();
	qWarning() << "data_feed_in(): sr_device->model() = "
		<< QString::fromStdString(sr_device->model())
		<< ", this->model() = " << QString::fromStdString(sr_device_->model());
	*/

	assert(sr_device);
	assert(sr_packet);

	if (sr_device != sr_device_)
		return;

	switch (sr_packet->type()->id()) {
	case SR_DF_HEADER:
		//qWarning() << "data_feed_in(): SR_DF_HEADER";
		feed_in_header();
		break;

	case SR_DF_META:
		//qWarning() << "data_feed_in(): SR_DF_META";
		feed_in_meta(
			dynamic_pointer_cast<sigrok::Meta>(sr_packet->payload()));
		break;

	case SR_DF_TRIGGER:
		//qWarning() << "data_feed_in(): SR_DF_TRIGGER";
		feed_in_trigger();
		break;

	case SR_DF_LOGIC:
		//qWarning() << "data_feed_in(): SR_DF_LOGIC";
		if (aquisition_state_ != AquisitionState::Running)
			return;

		try {
			feed_in_logic(
				dynamic_pointer_cast<sigrok::Logic>(sr_packet->payload()));
		} catch (bad_alloc &) {
			//out_of_memory_ = true;
		}
		break;

	case SR_DF_ANALOG:
		//qWarning() << "data_feed_in(): SR_DF_ANALOG";
		if (aquisition_state_ != AquisitionState::Running)
			return;

		try {
			feed_in_analog(
				dynamic_pointer_cast<sigrok::Analog>(sr_packet->payload()));
		} catch (bad_alloc &) {
			//out_of_memory_ = true;
		}
		break;

	case SR_DF_FRAME_BEGIN:
		//qWarning() << "data_feed_in(): SR_DF_FRAME_BEGIN";
		feed_in_frame_begin();
		break;

	case SR_DF_FRAME_END:
		//qWarning() << "data_feed_in(): SR_DF_FRAME_END";
		feed_in_frame_end();
		break;

	case SR_DF_END:
		//qWarning() << "data_feed_in(): SR_DF_END";
		// Strictly speaking, this is performed when a frame end marker was
		// received, so there's no point doing this again. However, not all
		// devices use frames, and for those devices, we need to do it here.
		{
			lock_guard<recursive_mutex> lock(data_mutex_);
		}
		break;

	default:
		break;
	}
}

void BaseDevice::aquisition_thread_proc()
{
	try {
		sr_session_->start();
	}
	catch (sigrok::Error &e) {
		Q_EMIT device_error(name(), e.what());
		return;
	}

	aquisition_state_ = AquisitionState::Running;
	/*
	// TODO: use std::chrono / std::time
	// NOTE: ATM only the session start timestamp is used!
	aquisition_start_timestamp_ =
		QDateTime::currentMSecsSinceEpoch() / (double)1000;
	Q_EMIT aquisition_start_timestamp_changed(aquisition_start_timestamp_);
	*/

	qWarning()
		<< "Start aquisition for " << short_name()
		<< ",  aquisition_start_timestamp_ = "
		<< util::format_time_date(aquisition_start_timestamp_);

	try {
		sr_session_->run();
	}
	catch (sigrok::Error &e) {
		Q_EMIT device_error(name(), e.what());
		aquisition_state_ = AquisitionState::Stopped;
		return;
	}
	aquisition_state_ = AquisitionState::Stopped;
}

} // namespace devices
} // namespace sv
