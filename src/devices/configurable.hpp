/*
 * This file is part of the SmuView project.
 *
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

#ifndef DEVICES_CONFIGURABLE_HPP
#define DEVICES_CONFIGURABLE_HPP

#include <memory>
#include <set>
#include <tuple>
#include <utility>

#include <QObject>
#include <QString>
#include <QStringList>

#include "src/data/datautil.hpp"
#include "src/devices/deviceutil.hpp"

using std::map;
using std::pair;
using std::set;
using std::shared_ptr;
using std::tuple;
using std::vector;

namespace sigrok {
class ConfigKey;
class Configurable;
class Quantity;
class QuantityFlag;
}

namespace sv {

namespace devices {

class Configurable : public QObject
{
	Q_OBJECT

public:
	Configurable(const shared_ptr<sigrok::Configurable> sr_configurable,
		const QString device_name);

	typedef pair<data::Quantity, set<data::QuantityFlag>>
		measured_quantity_t;
	typedef map<data::Quantity, vector<set<data::QuantityFlag>>>
		measured_quantity_list_t;

	bool has_get_config(devices::ConfigKey key) const;
	template<typename T> T get_config(devices::ConfigKey key) const;

	bool has_set_config(devices::ConfigKey key) const;
	template<typename T> void set_config(devices::ConfigKey key, const T value);

	bool has_list_config(devices::ConfigKey key) const;
	void list_config_string_array(devices::ConfigKey key,
		QStringList &string_list);
	void list_config_min_max_steps(devices::ConfigKey key,
		double &min, double &max, double &step);
	void list_config_mq(devices::ConfigKey key,
		measured_quantity_list_t &measured_quantity_list);

	QString name() const;

	set<devices::ConfigKey> available_getable_config_keys() const;
	set<devices::ConfigKey> available_setable_config_keys() const;
	set<devices::ConfigKey> available_listable_config_keys() const;

	bool is_controllable() const;

	/*
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

	bool is_otp_active_getable() const;
	bool get_otp_active() const;

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
	*/
	measured_quantity_t get_measured_quantity() const;
	void set_measured_quantity(measured_quantity_t measured_quantity);

	/*
	bool is_amplitude_getable() const;
	bool is_amplitude_setable() const;
	bool is_amplitude_listable() const;
	double get_amplitude() const;
	void set_amplitude(double amplitude);

	bool is_offset_getable() const;
	bool is_offset_setable() const;
	bool is_offset_listable() const;
	double get_offset() const;
	void set_offset(double offset);

	bool list_regulation(QStringList &regulation_list);
	bool list_voltage_target(double &min, double &max, double &step);
	bool list_current_limit(double &min, double &max, double &step);
	bool list_ovp_threshold(double &min, double &max, double &step);
	bool list_ocp_threshold(double &min, double &max, double &step);
	bool list_uvc_threshold(double &min, double &max, double &step);
	bool list_measured_quantity(
		measured_quantity_list_t &measured_quantity_list);
	bool list_amplitude(double &min, double &max, double &step);
	bool list_offset(double &min, double &max, double &step);
	*/

private:
	void init_properties();
	void init_values();

	const shared_ptr<sigrok::Configurable> sr_configurable_;
	const QString device_name_;

	/* TODO: remove?
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
	measured_quantity_list_t measured_quantity_list_;
	double amplitude_min_;
	double amplitude_max_;
	double amplitude_step_;
	double offset_min_;
	double offset_max_;
	double offset_step_;

	set<devices::ConfigKey> available_getable_config_keys_;
	set<devices::ConfigKey> available_setable_config_keys_;
	set<devices::ConfigKey> available_listable_config_keys_;

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
	bool is_amplitude_getable_;
	bool is_amplitude_setable_;
	bool is_amplitude_listable_;
	bool is_offset_getable_;
	bool is_offset_setable_;
	bool is_offset_listable_;

Q_SIGNALS:
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
	void amplitude_changed(const double);
	void offset_changed(const double);

};

} // namespace devices
} // namespace sv

#endif // DEVICES_CONFIGURABLE_HPP
