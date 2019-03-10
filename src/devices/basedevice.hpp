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

#ifndef DEVICES_BASEDEVICE_HPP
#define DEVICES_BASEDEVICE_HPP

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <QObject>
#include <QString>

#include "src/devices/deviceutil.hpp"

using std::function;
using std::map;
using std::mutex;
using std::recursive_mutex;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sigrok {
class Analog;
class Channel;
class Context;
class Device;
class Logic;
class Meta;
class Packet;
class Session;
}

namespace sv {

class DeviceManager;

namespace channels {
class BaseChannel;
}

namespace data {
class BaseSignal;
}

namespace devices {

class Configurable;

enum class AquisitionState {
	Stopped,
	Running,
	Paused
};

class BaseDevice :
	public QObject,
	public std::enable_shared_from_this<BaseDevice>
{
	Q_OBJECT

public:
	BaseDevice(const shared_ptr<sigrok::Context> sr_context,
		shared_ptr<sigrok::Device> sr_device);
	virtual ~BaseDevice();

	/**
	 * Inits all configurables for this device. Implemented in the
	 * specific device.
	 */
	virtual void init() = 0;

	/**
	 *
	 */
	shared_ptr<sigrok::Device> sr_device() const;

	/**
	 * Returns the device type
	 */
	DeviceType type() const;

	/**
	 * Get the unique Id of the device
	 */
	virtual string id() const = 0;

	/**
	 * Builds the name
	 */
	virtual string name() const;

	/**
	 * Builds the full name. It only contains all the fields.
	 */
	virtual QString full_name() const;

	/**
	 * Builds the short name.
	 */
	virtual QString short_name() const;

	/**
	 * Builds the display name. It only contains fields as required.
	 * @param device_manager a reference to the device manager is needed
	 * so that other similarly titled devices can be detected.
	 */
	virtual QString display_name(
		const DeviceManager &device_manager) const = 0;

	/**
	 * Open the device.
	 */
	virtual void open(function<void (const QString)> error_handler);

	/**
	 * Close the device.
	 */
	virtual void close();

	/**
	 * Start data aquisition from device after init or pause.
	 */
	virtual void start_aquisition();

	/**
	 * Pause data aquisition from device.
	 */
	virtual void pause_aquisition();

	/**
	 * Get the aquisition state.
	 */
	AquisitionState aquisition_state();

	/**
	 * Returns a map with all configurables of this device
	 */
	map<string, shared_ptr<devices::Configurable>> configurables() const;

	/**
	 * Add a sv::channels:.Channel to the device
	 */
	virtual void add_channel(shared_ptr<channels::BaseChannel> channel,
		string channel_group_name);

	/**
	 * Add a sigrok::Channel to the device.
	 */
	shared_ptr<channels::BaseChannel> add_sr_channel(
		shared_ptr<sigrok::Channel> sr_channel, string channel_group_name);

	/**
	 * Add an user channel to the device
	 */
	shared_ptr<channels::BaseChannel> add_user_channel(
		string channel_name, string channel_group_name,
		data::Quantity quantity, set<data::QuantityFlag> quantity_flags,
		data::Unit unit);

	/**
	 * Returns a map with all channels of this device
	 */
	map<string, shared_ptr<channels::BaseChannel>> channel_name_map() const;

	/**
	 * Returns a map with all channel groups of this device
	 */
	map<string, vector<shared_ptr<channels::BaseChannel>>> channel_group_name_map() const;

	/**
	 * Get the map which maps sigrok::Channel and sv::Channels::BaseChannel
	 */
	map<shared_ptr<sigrok::Channel>, shared_ptr<channels::BaseChannel>> sr_channel_map() const;

	/**
	 * Returns all signals of this device
	 */
	vector<shared_ptr<data::BaseSignal>> all_signals() const;


protected:
	/**
	 * Inits all channles for this device. Implemented in the specific device.
	 */
	virtual void init_channels() = 0;

	virtual void feed_in_header() = 0;
	virtual void feed_in_trigger() = 0;
	virtual void feed_in_meta(shared_ptr<sigrok::Meta> sr_meta) = 0;
	virtual void feed_in_frame_begin() = 0;
	virtual void feed_in_frame_end() = 0;
	virtual void feed_in_logic(shared_ptr<sigrok::Logic> sr_logic) = 0;
	virtual void feed_in_analog(shared_ptr<sigrok::Analog> sr_analog) = 0;

	void data_feed_in(shared_ptr<sigrok::Device> sr_device,
		shared_ptr<sigrok::Packet> sr_packet);

	const shared_ptr<sigrok::Context> sr_context_;
	shared_ptr<sigrok::Session> sr_session_;
	shared_ptr<sigrok::Device> sr_device_;
	DeviceType device_type_;
	bool device_open_;

	map<string, shared_ptr<devices::Configurable>> configurables_;

	map<string, shared_ptr<channels::BaseChannel>> channel_name_map_;
	map<string, vector<shared_ptr<channels::BaseChannel>>> channel_group_name_map_;
	map<shared_ptr<sigrok::Channel>, shared_ptr<channels::BaseChannel>> sr_channel_map_;
	vector<shared_ptr<data::BaseSignal>> all_signals_; // TODO: Is empty! Fill in Hardware/Virtual!

	mutable mutex aquisition_mutex_; //!< Protects access to capture_state_. // TODO
	mutable recursive_mutex data_mutex_; // TODO
	AquisitionState aquisition_state_;
	double aquisition_start_timestamp_;

	bool frame_began_;

private:
	void aquisition_thread_proc(function<void (const QString)> error_handler);

	function<void (const QString)> aquisition_thread_error_handler_;
	std::thread aquisition_thread_;

Q_SIGNALS:
	void aquisition_start_timestamp_changed(double);
	void channel_added(shared_ptr<sv::channels::BaseChannel>);

};

} // namespace devices
} // namespace sv

#endif // DEVICES_BASEDEVICE_HPP
