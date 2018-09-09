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

#ifndef DEVICES_PROPERTIES_BASEPROPERTY_HPP
#define DEVICES_PROPERTIES_BASEPROPERTY_HPP

#include <memory>

#include <glib.h>

#include <QObject>
#include <QVariant>

#include "src/devices/deviceutil.hpp"

using std::shared_ptr;

namespace sv {
namespace devices {

class Configurable;

namespace properties {

class BaseProperty : public QObject
{
	Q_OBJECT

public:
	BaseProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key);

	shared_ptr<devices::Configurable> configurable() const;
	devices::ConfigKey config_key() const;
	bool is_getable() const;
	bool is_setable() const;
	bool is_listable() const;
	virtual QVariant value() const = 0;

protected:
	shared_ptr<devices::Configurable> configurable_;
	devices::ConfigKey config_key_;
	bool is_getable_;
	bool is_setable_;
	bool is_listable_;

public Q_SLOTS:
	/**
	 * Value has changes within SmuView and should be send to the device
	 */
	virtual void change_value(const QVariant) = 0;
	/**
	 * Devices has sended a changed value via a meta package
	 */
	virtual void on_value_changed(Glib::VariantBase) = 0;

Q_SIGNALS:
	void value_changed(const QVariant);

};

} // namespace properties
} // namespace devices
} // namespece sv

#endif // DEVICES_PROPERTIES_BASEPROPERTY_HPP
