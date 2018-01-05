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
#include <set>
#include <string>
#include <thread>
#include <unordered_set>

#include <QString>

using std::function;
using std::map;
using std::mutex;
using std::recursive_mutex;
using std::set;
using std::shared_ptr;
using std::string;
using std::unordered_set;
using std::vector;

namespace sigrok {
class Analog;
class Channel;
class ChannelGroup;
class ConfigKey;
class Configurable;
class Context;
class Device;
class Quantity;
class QuantityFlag;
class Meta;
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

class Configurable;

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
	map<QString, shared_ptr<sigrok::ChannelGroup>> sr_channel_group_name_map_;
	map<QString, vector<shared_ptr<data::BaseSignal>>> channel_group_name_signals_map_;

	/**
	 * Mapping of incomming data to BaseSignal
	 */
	map<shared_ptr<sigrok::Channel>, shared_ptr<data::BaseSignal>> sr_channel_signal_map_;

	vector<shared_ptr<devices::Configurable>> configurables_;
	map<QString, shared_ptr<data::BaseSignal>> signal_name_map_;

	std::thread aquisition_thread_;
	mutable mutex aquisition_mutex_; //!< Protects access to capture_state_.
	aquisition_state aquisition_state_;
	bool out_of_memory_;
	bool frame_began_;

	virtual void init_device();
	virtual shared_ptr<data::BaseSignal> init_signal(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<data::AnalogData> common_time_data,
		bool quantity_fixed) = 0;

Q_SIGNALS:
	/*
	void capture_state_changed(int);
	void enabled_changed(const bool);
	void voltage_target_changed(const double);
	void current_limit_changed(const double);
	void otp_enabled_changed(const bool);
	void otp_active_changed(const bool);
	void ovp_enabled_changed(const bool);
	void ovp_active_changed(const bool);
	void ovp_threshold_changed(const double);
	void ocp_enabled_changed(const bool);
	void ocp_active_changed(const bool);
	void ocp_threshold_changed(const double);
	void uvc_enabled_changed(const bool);
	void uvc_active_changed(const bool);
	void uvc_threshold_changed(const double);
	*/

	/* TODO?
	void signals_changed();
	void device_changed();
	*/
};

} // namespace devices
} // namespace sv

#endif // DEVICES_DEVICE_HPP
