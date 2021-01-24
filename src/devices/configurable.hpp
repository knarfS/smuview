/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEVICES_CONFIGURABLE_HPP
#define DEVICES_CONFIGURABLE_HPP

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <glib.h>

#include <QObject>
#include <QString>
#include <QVariant>

#include "src/data/datautil.hpp"
#include "src/devices/deviceutil.hpp"

using std::forward;
using std::make_shared;
using std::map;
using std::pair;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sigrok {
class ConfigKey;
class Configurable;
class Quantity;
class QuantityFlag;
class Meta;
}

namespace sv {

namespace data {
namespace properties {
class BaseProperty;
}
}

namespace devices {

class Configurable :
	public QObject,
	public std::enable_shared_from_this<Configurable>
{
	Q_OBJECT

private:
	Configurable(const shared_ptr<sigrok::Configurable> sr_configurable,
		unsigned int configurable_index,
		const string &device_name, const DeviceType device_type,
		const QString &device_settings_id);

public:
	template<typename ...Arg>
	shared_ptr<Configurable> static create(Arg&&...arg)
	{
		struct make_shared_enabler : public Configurable {
			explicit make_shared_enabler(Arg&&...arg) :
				Configurable(forward<Arg>(arg)...)
			{
			}
		};

		shared_ptr<Configurable> configurable =
			make_shared<make_shared_enabler>(forward<Arg>(arg)...);
		configurable->init();

		return configurable;
	}


	/**
	 * Init the properties (config keys) and default lists.
	 * Must be called after instantiation and not from the ctor.
	 */
	void init();

	bool has_get_config(devices::ConfigKey config_key) const;
	template<typename T> T get_config(devices::ConfigKey config_key) const;
	/**
	 * Special handling for Container Variants (especially std::tuple, used for
	 * measured quantity, ranges and rationales).
	 * Tuple types are only supported with version >= 2.52 of glibmm, but we
	 * need to use version 2.42, because of mxe. Maybe there is also a bug in
	 * the tuple support?
	 *
	 * TODO: Remove when glibmm >= 2.52
	 */
	Glib::VariantContainerBase get_container_config(devices::ConfigKey config_key) const;
	/**
	 * Helper function to map to get_container_config().
	 *
	 * TODO: Remove when glibmm >= 2.52
	 */
	data::measured_quantity_t get_measured_quantity_config(
		devices::ConfigKey config_key) const;

	bool has_set_config(devices::ConfigKey config_key) const;
	template<typename T> void set_config(devices::ConfigKey config_key, const T value);
	/**
	 * Special handling for Container Variants (especially std::tuple, used for
	 * measured quantity, ranges and rationales).
	 * Tuple types are only supported with version >= 2.52 of glibmm, but we
	 * need to use version 2.42, because of mxe. Maybe there is also a bug in
	 * the tuple support?
	 *
	 * TODO: Remove when glibmm >= 2.52
	 */
	void set_container_config(devices::ConfigKey config_key,
		const vector<Glib::VariantBase> &childs);
	/**
	 * Helper function to map to set_container_config().
	 *
	 * TODO: Remove when glibmm >= 2.52
	 */
	void set_measured_quantity_config(devices::ConfigKey config_key,
		const data::measured_quantity_t mq);

	bool has_list_config(devices::ConfigKey config_key) const;
	bool list_config(devices::ConfigKey config_key, Glib::VariantContainerBase &gvar);

	/**
	 * Get the name of this configurable.
	 */
	string name() const;

	/**
	 * Get the display name of this configurable.
	 */
	QString display_name() const;

	/**
	 * Get the unique index number of this configurable.
	 */
	unsigned int index() const;

	/**
	 * Get the type of this configurable.
	 */
	DeviceType device_type() const;

	/**
	 * Get the settings id of the device this configurable belongs to.
	 */
	QString device_settings_id() const;

	set<devices::ConfigKey> getable_configs() const;
	set<devices::ConfigKey> setable_configs() const;
	set<devices::ConfigKey> listable_configs() const;

	map<devices::ConfigKey, shared_ptr<data::properties::BaseProperty>> property_map() const;
	shared_ptr<data::properties::BaseProperty> get_property(devices::ConfigKey config_key) const;

	bool is_controllable() const;

	bool feed_in_meta(shared_ptr<sigrok::Meta> sr_meta);

private:
	const shared_ptr<sigrok::Configurable> sr_configurable_;
	unsigned int index_;
	const string device_name_;
	const DeviceType device_type_;
	const QString device_settings_id_;

	set<devices::ConfigKey> getable_configs_;
	set<devices::ConfigKey> setable_configs_;
	set<devices::ConfigKey> listable_configs_;
	map<devices::ConfigKey, shared_ptr<data::properties::BaseProperty>> property_map_;

Q_SIGNALS:
	void config_changed(
		const devices::ConfigKey config_key, const QVariant &qvar);

};

} // namespace devices
} // namespace sv

#endif // DEVICES_CONFIGURABLE_HPP
