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

#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <utility>

#include <glib.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "src/data/datautil.hpp"
#include "src/devices/deviceutil.hpp"

using std::forward;
using std::make_shared;
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
class Meta;
}

namespace sv {
namespace devices {

namespace properties {
class BaseProperty;
}

class Configurable :
	public QObject,
	public std::enable_shared_from_this<Configurable>
{
	Q_OBJECT

private:
	Configurable(const shared_ptr<sigrok::Configurable> sr_configurable,
		const QString device_name);

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
	bool list_config_min_max_step(devices::ConfigKey key,
		double &min, double &max, double &step);

	QString name() const;

	set<devices::ConfigKey> getable_configs() const;
	set<devices::ConfigKey> setable_configs() const;
	set<devices::ConfigKey> listable_configs() const;

	map<devices::ConfigKey, shared_ptr<properties::BaseProperty>> properties() const;
	shared_ptr<properties::BaseProperty> get_property(devices::ConfigKey) const;

	bool is_controllable() const;

	void feed_in_meta(shared_ptr<sigrok::Meta> sr_meta);

private:
	const shared_ptr<sigrok::Configurable> sr_configurable_;
	const QString device_name_;

	set<devices::ConfigKey> getable_configs_;
	set<devices::ConfigKey> setable_configs_;
	set<devices::ConfigKey> listable_configs_;
	map<devices::ConfigKey, shared_ptr<properties::BaseProperty>> properties_;

Q_SIGNALS:
	void config_changed(const devices::ConfigKey, const QVariant);

};

} // namespace devices
} // namespace sv

#endif // DEVICES_CONFIGURABLE_HPP
