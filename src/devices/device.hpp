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

#ifndef DEVICES_DEVICE_HPP
#define DEVICES_DEVICE_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <QString>

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
class AnalogSignal;
class BaseSignal;
}

namespace devices {

enum class DeviceType {
	POWER_SUPPLY,
	ELECTRONIC_LOAD,
	MULTIMETER,
	DEMO_DEV,
	UNKNOWN
};

class Device : public QObject
{
	Q_OBJECT

public:
	Device(const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::Device> sr_device);
	virtual ~Device();

	enum aquisition_state {
		Stopped,
		AwaitingTrigger,
		Running
	};

	shared_ptr<sigrok::Device> sr_device() const;

	/**
	 * Returns the device type
	 */
	DeviceType type() const;

	/**
	 * Builds the name
	 */
	virtual QString name() const = 0;

	/**
	 * Builds the full name. It only contains all the fields.
	 */
	virtual QString full_name() const = 0;

	/**
	 * Builds the short name.
	 */
	virtual QString short_name() const = 0;

	/**
	 * Builds the display name. It only contains fields as required.
	 * @param device_manager a reference to the device manager is needed
	 * so that other similarly titled devices can be detected.
	 */
	virtual QString display_name(
		const DeviceManager &device_manager) const = 0;

	void open(function<void (const QString)> error_handler);
	void close();

	// TODO: typdefs?
	// TODO: Doxy
	map<QString, shared_ptr<channels::BaseChannel>> channel_name_map() const;
	map<QString, vector<shared_ptr<channels::BaseChannel>>> channel_group_name_map() const;
	vector<shared_ptr<data::AnalogSignal>> all_signals() const;

	virtual void free_unused_memory();

protected:
	virtual void feed_in_header() = 0;
	virtual void feed_in_trigger() = 0;
	virtual void feed_in_meta(shared_ptr<sigrok::Meta> sr_meta) = 0;
	virtual void feed_in_frame_begin() = 0;
	virtual void feed_in_frame_end() = 0;
	virtual void feed_in_logic(shared_ptr<sigrok::Logic> sr_logic) = 0;
	virtual void feed_in_analog(shared_ptr<sigrok::Analog> sr_analog) = 0;

	void init_channel(shared_ptr<channels::BaseChannel> channel,
		QString channel_group_name);

	void data_feed_in(shared_ptr<sigrok::Device> sr_device,
		shared_ptr<sigrok::Packet> sr_packet);

	const shared_ptr<sigrok::Context> sr_context_;
	shared_ptr<sigrok::Session> sr_session_;
	shared_ptr<sigrok::Device> sr_device_;
	DeviceType device_type_;
	bool device_open_;

	// TODO: typdefs?
	// TODO: Doxy
	map<QString, shared_ptr<channels::BaseChannel>> channel_name_map_;
	map<QString, vector<shared_ptr<channels::BaseChannel>>> channel_group_name_map_;
	vector<shared_ptr<data::AnalogSignal>> all_signals_;

	mutable mutex aquisition_mutex_; //!< Protects access to capture_state_. // TODO
	mutable recursive_mutex data_mutex_; // TODO
	aquisition_state aquisition_state_;
	double aquisition_start_timestamp_;

	bool out_of_memory_;
	bool frame_began_;

private:
	void aquisition_thread_proc(function<void (const QString)> error_handler);

	std::thread aquisition_thread_;

Q_SIGNALS:
	void aquisition_start_timestamp_changed(double);

};

} // namespace devices
} // namespace sv

#endif // DEVICES_DEVICE_HPP
