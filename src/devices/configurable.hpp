/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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
		const string device_name, const DeviceType device_type);

public:
	template<typename ...Arg>
	shared_ptr<Configurable> static create(Arg&&...arg)
	{
		struct make_shared_enabler : public Configurable {
			make_shared_enabler(Arg&&...arg) : Configurable(forward<Arg>(arg)...) {}
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

	bool has_get_config(devices::ConfigKey) const;
	template<typename T> T get_config(devices::ConfigKey) const;
	/**
	 * Special handling for Conatiner Variants (especially std::tuple).
	 * Tuple types are only supported with version >= 2.52 of glibmm, but we
	 * need to use version 2.42, because of mxe.
	 */
	Glib::VariantContainerBase get_container_config(devices::ConfigKey) const;

	bool has_set_config(devices::ConfigKey) const;
	template<typename T> void set_config(devices::ConfigKey, const T);
	/**
	 * Special handling for Conatiner Variants (especially std::tuple).
	 * Tuple types are only supported with version >= 2.52 of glibmm, but we
	 * need to use version 2.42, because of mxe.
	 */
	void set_container_config(devices::ConfigKey, vector<Glib::VariantBase>);

	bool has_list_config(devices::ConfigKey) const;
	bool list_config(devices::ConfigKey, Glib::VariantContainerBase &);

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

	set<devices::ConfigKey> getable_configs() const;
	set<devices::ConfigKey> setable_configs() const;
	set<devices::ConfigKey> listable_configs() const;

	map<devices::ConfigKey, shared_ptr<data::properties::BaseProperty>> properties() const;
	shared_ptr<data::properties::BaseProperty> get_property(devices::ConfigKey) const;

	bool is_controllable() const;

	void feed_in_meta(shared_ptr<sigrok::Meta>);

private:
	const shared_ptr<sigrok::Configurable> sr_configurable_;
	unsigned int configurable_index_;
	const string device_name_;
	const DeviceType device_type_;

	set<devices::ConfigKey> getable_configs_;
	set<devices::ConfigKey> setable_configs_;
	set<devices::ConfigKey> listable_configs_;
	map<devices::ConfigKey, shared_ptr<data::properties::BaseProperty>> properties_;

Q_SIGNALS:
	void config_changed(const devices::ConfigKey, const QVariant);

};

} // namespace devices
} // namespace sv

#endif // DEVICES_CONFIGURABLE_HPP
