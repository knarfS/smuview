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

#ifndef DEVICES_HARDWAREDEVICE_HPP
#define DEVICES_HARDWAREDEVICE_HPP

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <unordered_set>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QString>

#include "src/devices/basedevice.hpp"

using std::bad_alloc;
using std::dynamic_pointer_cast;
using std::function;
using std::lock_guard;
using std::make_shared;
using std::set;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::vector;
using std::unique_ptr;

using std::function;
using std::map;
using std::mutex;
using std::recursive_mutex;
using std::set;
using std::shared_ptr;
using std::string;
using std::unordered_set;

namespace sigrok {
class Channel;
class Configurable;
class Context;
class Quantity;
class HardwareDevice;
}

namespace sv {

namespace channels {
class BaseChannel;
}

namespace data {
class AnalogSignal;
class BaseSignal;
}

namespace devices {

namespace properties {
class UInt64Property;
}

class Configurable;

class HardwareDevice : public BaseDevice
{
	Q_OBJECT

protected:
	HardwareDevice(const shared_ptr<sigrok::Context> sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device);

public:
	/**
	 * Inits all configurables for this hardware device.
	 */
	void init() override;

	/**
	 * Returns the sigrok hardware device
	 */
	shared_ptr<sigrok::HardwareDevice> sr_hardware_device() const;

	/**
	 * Get the unique Id of the device
	 */
	string id() const override;

	/**
	 * Builds the display name. It only contains fields as required.
	 * @param device_manager a reference to the device manager is needed
	 * so that other similarly titled devices can be detected.
	 */
	QString display_name(const DeviceManager &device_manager) const override;

	void open(function<void (const QString)> error_handler) override;

	vector<shared_ptr<devices::Configurable>> configurables() const;
	// TODO: typdef?
	map<shared_ptr<sigrok::Channel>, shared_ptr<channels::BaseChannel>> sr_channel_map() const;

protected:
	/**
	 * Inits all channles of this hardware device
	 */
	void init_channels() override;

	/**
	 * Adds a channel to the device
	 */
	shared_ptr<channels::BaseChannel> init_channel(
		shared_ptr<sigrok::Channel> sr_channel, QString channel_group_name);

	void feed_in_header() override;
	void feed_in_trigger() override;
	void feed_in_meta(shared_ptr<sigrok::Meta> sr_meta) override;
	void feed_in_frame_begin() override;
	void feed_in_frame_end() override;
	void feed_in_logic(shared_ptr<sigrok::Logic> sr_logic) override;
	void feed_in_analog(shared_ptr<sigrok::Analog> sr_analog) override;

	vector<shared_ptr<devices::Configurable>> configurables_;
	// TODO: typdef?
	map<shared_ptr<sigrok::Channel>, shared_ptr<channels::BaseChannel>> sr_channel_map_;

private:
	double frame_start_timestamp_;
	shared_ptr<properties::UInt64Property> samplerate_prop_;

Q_SIGNALS:
	void channel_changed(QString);

};

} // namespace devices
} // namespace sv

#endif // DEVICES_HARDWAREDEVICE_HPP
