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
#include <string>
#include <thread>
#include <unordered_set>

using std::function;
using std::map;
using std::mutex;
using std::recursive_mutex;
using std::shared_ptr;
using std::string;
using std::unordered_set;
using std::vector;

namespace sigrok {
class ConfigKey;
class Context;
class Device;
class Packet;
class Session;
}

namespace sv {

class DeviceManager;

namespace data {
class AnalogData;
class BaseSignal;
class BaseData;
}

namespace devices {

class Device : public QObject
{
	Q_OBJECT

public:
	Device(const shared_ptr<sigrok::Context> &sr_context);
	virtual ~Device();

	enum aquisition_state {
		Stopped,
		AwaitingTrigger,
		Running
	};

	shared_ptr<sigrok::Device> sr_device() const;

	bool is_read_config(const sigrok::ConfigKey *key) const;
	template<typename T> T read_config(const sigrok::ConfigKey *key) const;

	bool is_write_config(const sigrok::ConfigKey *key) const;
	template<typename T> void write_config(const sigrok::ConfigKey *key,
		const T value);

	bool is_list_config(const sigrok::ConfigKey *key) const;
	template<typename T> T list_config(const sigrok::ConfigKey *key) const;

	/**
	 * Builds the full name. It only contains all the fields.
	 */
	virtual string full_name() const = 0;

	/**
	 * Builds the display name. It only contains fields as required.
	 * @param device_manager a reference to the device manager is needed
	 * so that other similarly titled devices can be detected.
	 */
	virtual string display_name(
		const DeviceManager &device_manager) const = 0;

	virtual void open(function<void (const QString)> error_handler) = 0;
	virtual void close() = 0;
	virtual void free_unused_memory();

	void data_feed_in(shared_ptr<sigrok::Device> sr_device,
		shared_ptr<sigrok::Packet> sr_packet);

private:
	mutable recursive_mutex data_mutex_;
	shared_ptr<data::BaseSignal> actual_processed_signal_;

	void feed_in_header();
	void feed_in_meta(shared_ptr<sigrok::Meta> sr_meta);
	void feed_in_trigger();
	void feed_in_frame_begin();
	void feed_in_frame_end();
	void feed_in_analog(shared_ptr<sigrok::Analog> sr_analog);

protected:
	const shared_ptr<sigrok::Context> sr_context_;
	shared_ptr<sigrok::Session> sr_session_;
	shared_ptr<sigrok::Device> sr_device_;
	shared_ptr<sigrok::Configurable> sr_configurable_;
	map<shared_ptr<sigrok::Channel>, shared_ptr<data::BaseSignal>> channel_data_; // TODO: Rename

	std::thread aquisition_thread_;
	mutable mutex aquisition_mutex_; //!< Protects access to capture_state_.
	aquisition_state aquisition_state_;
	bool out_of_memory_;
	bool frame_began_;

	virtual void init_device();
	virtual shared_ptr<data::BaseSignal> init_signal(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<data::AnalogData> common_time_data) = 0;

// TODO: move to hardwaredevice
Q_SIGNALS:
	void capture_state_changed(int);
	void enabled_changed(const bool);
	void voltage_target_changed(const double);
	void current_limit_changed(const double);
	void over_voltage_protection_active_changed(const bool);
	void over_current_protection_active_changed(const bool);
	void under_voltage_condition_active_changed(const bool);
	void under_voltage_condition_threshold_changed(const double);
	void over_temperature_protection_active_changed(const bool);

	/* TODO?
	void signals_changed();
	void device_changed();
	*/
};

} // namespace devices
} // namespace sv

#endif // DEVICES_DEVICE_HPP
