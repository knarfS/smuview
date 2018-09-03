/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <glib.h>

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
	bool list_config(devices::ConfigKey key,
		Glib::VariantContainerBase &gvariant);
	bool list_config_string_array(devices::ConfigKey key,
		QStringList &string_list);
	bool list_config_min_max_steps(devices::ConfigKey key,
		double &min, double &max, double &step);
	bool list_config_mq(devices::ConfigKey key,
		measured_quantity_list_t &measured_quantity_list);

	QString name() const;

	set<devices::ConfigKey> getable_configs() const;
	set<devices::ConfigKey> setable_configs() const;
	set<devices::ConfigKey> listable_configs() const;

	bool is_controllable() const;

	measured_quantity_t get_measured_quantity() const;
	void set_measured_quantity(measured_quantity_t measured_quantity);

private:
	void init_properties();
	void init_values();

	const shared_ptr<sigrok::Configurable> sr_configurable_;
	const QString device_name_;

	set<devices::ConfigKey> getable_configs_;
	set<devices::ConfigKey> setable_configs_;
	set<devices::ConfigKey> listable_configs_;

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
