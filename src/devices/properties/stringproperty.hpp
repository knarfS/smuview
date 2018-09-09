/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DEVICES_PROPERTIES_STRINGPROPERTY_HPP
#define DEVICES_PROPERTIES_STRINGPROPERTY_HPP

#include <map>
#include <memory>

#include <glib.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "src/devices/properties/baseproperty.hpp"
#include "src/devices/deviceutil.hpp"

using std::map;
using std::shared_ptr;

namespace sv {
namespace devices {

class Configurable;

namespace properties {

class StringProperty : public BaseProperty
{
	Q_OBJECT

public:
	StringProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key);

public:
	QVariant value() const;
	QString string_value() const;
	QStringList list_values() const;

private:
	QStringList string_list_;

	bool list_config();

public Q_SLOTS:
	void change_value(const QVariant);
	void on_value_changed(Glib::VariantBase);

};

} // namespace properties
} // namespace devices
} // namespece sv

#endif // DEVICES_PROPERTIES_STRINGPROPERTY_HPP
