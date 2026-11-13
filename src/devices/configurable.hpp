/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <glib.h>

#include <QObject>
#include <QString>
#include <QVariant>

#include "src/devices/deviceenums.hpp"
#include "src/devices/deviceref.hpp"

using std::map;
using std::set;
using std::shared_ptr;
using std::string;

namespace sigrok {
class Configurable;
class Meta;
}

namespace sv {

namespace data {
namespace properties {
class BaseProperty;
}
}

namespace devices {

class ConfigInterface;

class Configurable : public QObject
{
	Q_OBJECT

public:
	Configurable(const shared_ptr<sigrok::Configurable> sr_configurable,
		unsigned int configurable_index,
		const devices::DeviceRef &parent_device);

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

	bool has_get_config(devices::ConfigKey config_key) const;
	bool has_set_config(devices::ConfigKey config_key) const;
	bool has_list_config(devices::ConfigKey config_key) const;
	set<devices::ConfigKey> getable_configs() const;
	set<devices::ConfigKey> setable_configs() const;
	set<devices::ConfigKey> listable_configs() const;

	map<devices::ConfigKey, shared_ptr<data::properties::BaseProperty>>
	property_map() const;

	shared_ptr<data::properties::BaseProperty>
	get_property(devices::ConfigKey config_key) const;

	bool is_controllable() const;

	bool feed_in_meta(shared_ptr<sigrok::Meta> sr_meta);

	const shared_ptr<devices::ConfigInterface> &config_interface() const;

	/**
	 * Get the parent device reference of this configurable.
	 */
	const devices::DeviceRef &parent_device_ref() const;

private:
	/**
	 * Returns the name of this configurable.
	 */
	string init_name(const shared_ptr<sigrok::Configurable> &sr_configurable);

	/**
	 * Init the properties (config keys) and default lists.
	 */
	void init_properties();

	const string name_;
	const shared_ptr<sigrok::Configurable> sr_configurable_;
	const unsigned int index_;
	const devices::DeviceRef parent_device_ref_;
	const shared_ptr<ConfigInterface> config_interface_;

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
