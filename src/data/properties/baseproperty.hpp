/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_PROPERTIES_BASEPROPERTY_HPP
#define DATA_PROPERTIES_BASEPROPERTY_HPP

#include <memory>
#include <string>

#include <glib.h>

#include <QObject>
#include <QString>
#include <QVariant>

#include "src/data/datautil.hpp"
#include "src/devices/deviceutil.hpp"

using std::shared_ptr;
using std::string;

namespace sv {

namespace devices {
class Configurable;
}

namespace data {
namespace properties {

class BaseProperty : public QObject
{
	Q_OBJECT

public:
	BaseProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key);

	shared_ptr<devices::Configurable> configurable() const;
	devices::ConfigKey config_key() const;
	data::DataType data_type() const;
	//data::Quantity quantity() const;
	data::Unit unit() const;
	bool is_getable() const;
	bool is_setable() const;
	bool is_listable() const;
	string name() const;
	QString display_name() const;
	virtual QVariant value() const = 0;
	virtual QString to_string(const QVariant qvar) const = 0;
	virtual QString to_string() const = 0;

protected:
	shared_ptr<devices::Configurable> configurable_;
	devices::ConfigKey config_key_;
	data::DataType data_type_;
	//data::Quantity quantity_;
	data::Unit unit_;
	bool is_getable_;
	bool is_setable_;
	bool is_listable_;

public Q_SLOTS:
	/**
	 * Load the list of available values for this property.
	 */
	virtual bool list_config() = 0;
	/**
	 * Value has changed within SmuView and should be send to the device.
	 */
	virtual void change_value(const QVariant qvar) = 0;
	/**
	 * Devices has send a changed value via a meta package.
	 */
	virtual void on_value_changed(Glib::VariantBase gvar) = 0;

Q_SIGNALS:
	void value_changed(const QVariant qvar);
	void list_changed();

};

} // namespace properties
} // namespace data
} // namespece sv

#endif // DATA_PROPERTIES_BASEPROPERTY_HPP
