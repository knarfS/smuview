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

#include <map>
#include <mutex>
#include <unordered_set>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "device.hpp"

using std::bad_alloc;
using std::dynamic_pointer_cast;
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
using std::shared_ptr;
using std::string;
using std::unordered_set;

namespace sigrok {
class Channel;
class Configurable;
class Context;
class HardwareDevice;
}

namespace sv {

namespace data {
class AnalogSegment;
class SignalBase;
class SignalData;
}

namespace devices {

class HardwareDevice final : public Device
{
	Q_OBJECT

public:
	HardwareDevice(const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device);

	~HardwareDevice();

	enum Type {
		POWER_SUPPLY,
		ELECTRONIC_LOAD,
		MULTIMETER,
		UNKNOWN
	};

	shared_ptr<sigrok::HardwareDevice> sr_hardware_device() const;

	/**
	 * Builds the full name. It only contains all the fields.
	 */
	string full_name() const;

	/**
	 * Builds the display name. It only contains fields as required.
	 * @param device_manager a reference to the device manager is needed
	 * so that other similarly titled devices can be detected.
	 */
	string display_name(const DeviceManager &device_manager) const;

	void open();
	void close();

	// TODO: Generic and from the same typ!
	shared_ptr<data::SignalBase> voltage_signal() const;
	shared_ptr<data::SignalBase> current_signal() const;
	shared_ptr<data::SignalBase> measurement_signal() const;
	shared_ptr<data::Analog> time_data() const;

	void get_all_config_check();

	bool is_enable_getable() const;
	bool is_enable_setable() const;
	bool get_enabled() const;
	void set_enable(const bool enable);

	bool is_voltage_target_getable() const;
	bool is_voltage_target_setable() const;
	double get_voltage_target() const;
	void set_voltage_target(const double value);
	void list_voltage_target(double &min, double &max, double &step);

	bool is_current_limit_getable() const;
	bool is_current_limit_setable() const;
	double get_current_limit() const;
	void set_current_limit(const double value);
	void list_current_limit(double &min, double &max, double &step);

	bool is_over_voltage_active_getable() const;
	double get_over_voltage_active() const;

	bool is_over_current_active_getable() const;
	double get_over_current_active() const;

	bool is_over_temperature_active_getable() const;
	double get_over_temperature_active() const;

	bool is_under_voltage_enable_getable() const;
	bool is_under_voltage_enable_setable() const;
	double get_under_voltage_enable() const;

	bool is_under_voltage_active_getable() const;
	double get_under_voltage_active() const;

	bool is_under_voltage_threshold_getable() const;
	bool is_under_voltage_threshold_setable() const;
	double get_under_voltage_threshold() const;
	void set_under_voltage_threshold(const double value);
	void list_under_voltage_threshold(double &min, double &max, double &step);

	/*
	void data_feed_in(shared_ptr<sigrok::Device> sr_device,
		shared_ptr<sigrok::Packet> sr_packet);
	*/

private:
	const shared_ptr<sigrok::Context> sr_context_;
	shared_ptr<sigrok::Configurable> sr_configurable_;
	Type type_;
	bool device_open_;

/*
Q_SIGNALS:
	void enabled_changed(bool);
	void voltage_target_changed(double);
	void current_limit_changed(double);
	void over_voltage_protection_active_changed(bool);
	void over_current_protection_active_changed(bool);
	void under_voltage_condition_active_changed(bool);
	void over_temperature_protection_active_changed(bool);
*/
};

} // namespace devices
} // namespace sv

#endif // DEVICES_HARDWAREDEVICE_HPP
