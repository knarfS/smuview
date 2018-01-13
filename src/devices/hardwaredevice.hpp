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
class HardwareDevice;
}

namespace sv {

namespace data {
class AnalogSignal;
class BaseSignal;
}

namespace devices {

class Channel;
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
	 * Returns the device type
	 */
	HardwareDevice::Type type() const;

	/**
	 * Builds the name
	 */
	QString name() const;

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
	QString display_name(const DeviceManager &device_manager) const;

	// TODO: move all maps to Device, because aqcu is there?
	// TODO: typdefs or classes?
	/*
	map<QString, vector<shared_ptr<data::BaseSignal>>> cg_name_signals_map() const;
	map<shared_ptr<sigrok::Channel>, shared_ptr<data::BaseSignal>> sr_channel_signal_map() const;
	map<QString, map<const sigrok::Quantity *, shared_ptr<data::AnalogSignal>>> ch_name_sr_quantity_signals_map() const;
	map<QString, map<const sigrok::Quantity *, shared_ptr<data::AnalogSignal>>> cg_name_sr_quantity_signals_map() const;
	*/

	vector<shared_ptr<devices::Configurable>> configurables() const;

	map<QString, shared_ptr<devices::Channel>> channel_name_map() const;
	map<shared_ptr<sigrok::Channel>, shared_ptr<devices::Channel>> sr_channel_map() const;
	map<QString, vector<shared_ptr<devices::Channel>>> channel_group_name_map() const;

	vector<shared_ptr<data::AnalogSignal>> all_signals() const;

protected:
	shared_ptr<devices::Channel> init_channel(
		shared_ptr<sigrok::Channel> sr_channel, QString channel_group_name);

	void feed_in_header();
	void feed_in_trigger();
	void feed_in_frame_begin();
	void feed_in_frame_end();
	void feed_in_logic(shared_ptr<sigrok::Logic> sr_logic);
	void feed_in_analog(shared_ptr<sigrok::Analog> sr_analog);

	/*
	void add_signal_to_maps(shared_ptr<data::AnalogSignal> signal,
		shared_ptr<sigrok::Channel> sr_channel, QString channel_group_name);
	*/

	Type type_;

	// TODO: move all maps to Device, because aqcu is there?
	// TODO: typdefs or classes?
	/*
	map<QString, vector<shared_ptr<data::BaseSignal>>> cg_name_signals_map_;
	/ **
	 * Mapping of incomming data to the actual BaseSignal (Quantity)
	 * /
	map<shared_ptr<sigrok::Channel>, shared_ptr<data::BaseSignal>> sr_channel_signal_map_; // TODO: Name: sr_channel_actual_signal_
	/ **
	 * Helps to seperate quantity and channel (for Dmms, etc.)
	 * /
	map<QString, map<const sigrok::Quantity *, shared_ptr<data::AnalogSignal>>> ch_name_sr_quantity_signals_map_; // TODO: Name
	/ **
	 * Helps to find V and I signals for calculation P, etc. (PSU and Loads)
	 * /
	map<QString, map<const sigrok::Quantity *, shared_ptr<data::AnalogSignal>>> cg_name_sr_quantity_signals_map_; // TODO: Name
	*/

	vector<shared_ptr<devices::Configurable>> configurables_;

	map<QString, shared_ptr<devices::Channel>> channel_name_map_;
	map<shared_ptr<sigrok::Channel>, shared_ptr<devices::Channel>> sr_channel_map_;
	map<QString, vector<shared_ptr<devices::Channel>>> channel_group_name_map_;

	vector<shared_ptr<data::AnalogSignal>> all_signals_;

private:
	double frame_start_timestamp_;

Q_SIGNALS:
	void channel_changed(QString);

};

} // namespace devices
} // namespace sv

#endif // DEVICES_HARDWAREDEVICE_HPP
