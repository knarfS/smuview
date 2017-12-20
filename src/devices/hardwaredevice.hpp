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
}

namespace sv {

namespace data {
class AnalogData;
class BaseSignal;
class BaseData;
}

namespace devices {

class HardwareDevice final : public Device
{
	Q_OBJECT

public:
	explicit HardwareDevice(const shared_ptr<sigrok::Context> &sr_context,
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
	string full_name() const;

	/**
	 * Builds the display name. It only contains fields as required.
	 * @param device_manager a reference to the device manager is needed
	 * so that other similarly titled devices can be detected.
	 */
	string display_name(const DeviceManager &device_manager) const;

	void open(function<void (const QString)> error_handler);
	void close();

	// TODO: Generic!
	shared_ptr<data::BaseSignal> voltage_signal() const;
	shared_ptr<data::BaseSignal> current_signal() const;
	shared_ptr<data::BaseSignal> measurement_signal() const;
	vector<shared_ptr<data::BaseSignal>> all_signals() const;

	bool is_controllable() const;

	bool is_enabled_getable() const;
	bool is_enabled_setable() const;
	bool get_enabled() const;
	void set_enabled(const bool enabled);

	bool is_regulation_getable() const;
	bool is_regulation_setable() const;
	bool get_regulation() const;
	void set_regulation(const bool regulation);

	bool is_voltage_target_getable() const;
	bool is_voltage_target_setable() const;
	bool is_voltage_target_listable() const;
	double get_voltage_target() const;
	void set_voltage_target(const double value);

	bool is_current_limit_getable() const;
	bool is_current_limit_setable() const;
	bool is_current_limit_listable() const;
	double get_current_limit() const;
	void set_current_limit(const double value);

	bool is_ovp_enabled_getable() const;
	bool is_ovp_enabled_setable() const;
	bool get_ovp_enabled() const;
	void set_ovp_enabled(const bool enabled);
	bool is_ovp_active_getable() const;
	bool get_ovp_active() const;
	bool is_ovp_threshold_getable() const;
	bool is_ovp_threshold_setable() const;
	bool is_ovp_threshold_listable() const;
	double get_ovp_threshold() const;
	void set_ovp_threshold(const double threshold);

	bool is_ocp_enabled_getable() const;
	bool is_ocp_enabled_setable() const;
	bool get_ocp_enabled() const;
	void set_ocp_enabled(const bool enabled);
	bool is_ocp_active_getable() const;
	bool get_ocp_active() const;
	bool is_ocp_threshold_getable() const;
	bool is_ocp_threshold_setable() const;
	bool is_ocp_threshold_listable() const;
	double get_ocp_threshold() const;
	void set_ocp_threshold(const double threshold);

	bool is_otp_active_getable() const;
	bool get_otp_active() const;

	bool is_uvc_enabled_getable() const;
	bool is_uvc_enabled_setable() const;
	bool get_uvc_enabled() const;
	void set_uvc_enabled(const bool enabled);
	bool is_uvc_active_getable() const;
	bool get_uvc_active() const;
	bool is_uvc_threshold_getable() const;
	bool is_uvc_threshold_setable() const;
	bool is_uvc_threshold_listable() const;
	double get_uvc_threshold() const;
	void set_uvc_threshold(const double threshold);

	bool is_measured_quantity_getable() const;
	bool is_measured_quantity_setable() const;
	bool is_measured_quantity_listable() const;
	void get_measured_quantity() const;
	void set_measured_quantity(uint mq, uint mq_flags);

	bool list_regulation(QStringList &regulation_list);
	bool list_voltage_target(double &min, double &max, double &step);
	bool list_current_limit(double &min, double &max, double &step);
	bool list_ovp_threshold(double &min, double &max, double &step);
	bool list_ocp_threshold(double &min, double &max, double &step);
	bool list_uvc_threshold(double &min, double &max, double &step);
	bool list_measured_quantity(sr_mq_flags_list_t &sr_mq_flags_list,
		mq_flags_list_t &mq_flags_list);

private:
	Type type_;
	bool device_open_;

	void init_device_properties();
	void init_device_values();
	void aquisition_thread_proc(function<void (const QString)> error_handler);

	// TODO: Generic!
	shared_ptr<data::BaseSignal> voltage_signal_;
	shared_ptr<data::BaseSignal> current_signal_;
	shared_ptr<data::BaseSignal> measurement_signal_;
	vector<shared_ptr<data::BaseSignal>> all_signals_;

	shared_ptr<data::AnalogData> init_time_data();
	shared_ptr<data::BaseSignal> init_signal(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<data::AnalogData> common_time_data);

	/*
	bool enabled_;
	QString regulation_;
	double voltage_target_;
	double current_limit_;
	bool otp_enabled_;
	bool otp_active_;
	bool ovp_enabled_;
	bool ovp_active_;
	double ovp_threshold_;
	bool ocp_enabled_;
	bool ocp_active_;
	double ocp_threshold_;
	bool uvc_enabled_;
	bool uvc_active_;
	double uvc_threshold_;
	*/

	QStringList regulation_list_;
	double voltage_target_min_;
	double voltage_target_max_;
	double voltage_target_step_;
	double current_limit_min_;
	double current_limit_max_;
	double current_limit_step_;
	double ovp_threshold_min_;
	double ovp_threshold_max_;
	double ovp_threshold_step_;
	double ocp_threshold_min_;
	double ocp_threshold_max_;
	double ocp_threshold_step_;
	double uvc_threshold_min_;
	double uvc_threshold_max_;
	double uvc_threshold_step_;
	// TODO: Find a better way to map MQ and MQ_FLAGS
	sr_mq_flags_list_t sr_mq_flags_list_;
	mq_flags_list_t mq_flags_list_;

	bool is_enabled_getable_;
	bool is_enabled_setable_;
	bool is_regulation_getable_;
	bool is_regulation_setable_;
	bool is_regulation_listable_;
	bool is_voltage_target_getable_;
	bool is_voltage_target_setable_;
	bool is_voltage_target_listable_;
	bool is_current_limit_getable_;
	bool is_current_limit_setable_;
	bool is_current_limit_listable_;
	bool is_otp_enabled_getable_;
	bool is_otp_enabled_setable_;
	bool is_otp_active_getable_;
	bool is_otp_active_setable_;
	bool is_ovp_enabled_getable_;
	bool is_ovp_enabled_setable_;
	bool is_ovp_active_getable_;
	bool is_ovp_active_setable_;
	bool is_ovp_threshold_getable_;
	bool is_ovp_threshold_setable_;
	bool is_ovp_threshold_listable_;
	bool is_ocp_enabled_getable_;
	bool is_ocp_enabled_setable_;
	bool is_ocp_active_getable_;
	bool is_ocp_active_setable_;
	bool is_ocp_threshold_getable_;
	bool is_ocp_threshold_setable_;
	bool is_ocp_threshold_listable_;
	bool is_uvc_enabled_getable_;
	bool is_uvc_enabled_setable_;
	bool is_uvc_active_getable_;
	bool is_uvc_active_setable_;
	bool is_uvc_threshold_getable_;
	bool is_uvc_threshold_setable_;
	bool is_uvc_threshold_listable_;
	bool is_measured_quantity_getable_;
	bool is_measured_quantity_setable_;
	bool is_measured_quantity_listable_;

Q_SIGNALS:
	/*
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
};

} // namespace devices
} // namespace sv

#endif // DEVICES_HARDWAREDEVICE_HPP
