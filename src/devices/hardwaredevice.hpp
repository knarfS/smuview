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

#ifndef DEVICES_HARDWAREDEVICE_HPP
#define DEVICES_HARDWAREDEVICE_HPP

#include <functional>
#include <map>
#include <mutex>
#include <unordered_set>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QString>
#include <QStringList>

#include "device.hpp"

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
class QuantityFlag;
class HardwareDevice;
class Meta;
}

namespace sv {

namespace data {
class AnalogSignal;
class BaseSignal;
}

namespace devices {

class Configurable;

class HardwareDevice : public Device
{
	Q_OBJECT

public:
	HardwareDevice(const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device);

	~HardwareDevice();

	// TODO: use sigrok ConfigKey?
	enum Type {
		POWER_SUPPLY,
		ELECTRONIC_LOAD,
		MULTIMETER,
		DEMO_DEV,
		UNKNOWN
	};

	shared_ptr<sigrok::HardwareDevice> sr_hardware_device() const;

	/**
	 * Builds the full name. It only contains all the fields.
	 */
	QString full_name() const;

	/**
	 * Builds the short name.
	 */
	QString short_name() const;

	/**
	 * Builds the display name. It only contains fields as required.
	 * @param device_manager a reference to the device manager is needed
	 * so that other similarly titled devices can be detected.
	 */
	string display_name(const DeviceManager &device_manager) const;

	vector<shared_ptr<data::AnalogSignal>> all_signals() const;
	map<QString, vector<shared_ptr<data::BaseSignal>>> channel_group_name_signals_map() const;
	vector<shared_ptr<devices::Configurable>> configurables() const;

protected:
	void feed_in_header();
	void feed_in_trigger();
	void feed_in_frame_begin();
	void feed_in_frame_end();
	void feed_in_logic(shared_ptr<sigrok::Logic> sr_logic);
	void feed_in_analog(shared_ptr<sigrok::Analog> sr_analog);

	Type type_;

	/**
	 * Mapping of incomming data to BaseSignal
	 */
	map<shared_ptr<sigrok::Channel>, shared_ptr<data::BaseSignal>> sr_channel_signal_map_;
	map<QString, vector<shared_ptr<data::BaseSignal>>> channel_group_name_signals_map_;
	map<QString, shared_ptr<data::BaseSignal>> signal_name_map_;
	vector<shared_ptr<data::AnalogSignal>> all_signals_;

	vector<shared_ptr<devices::Configurable>> configurables_;

private:
	shared_ptr<data::BaseSignal> actual_processed_signal_;

};

} // namespace devices
} // namespace sv

#endif // DEVICES_HARDWAREDEVICE_HPP
