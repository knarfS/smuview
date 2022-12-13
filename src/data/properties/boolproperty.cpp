/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QString>
#include <QVariant>

#include "boolproperty.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace data {
namespace properties {

BoolProperty::BoolProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key)
{
}

QVariant BoolProperty::value() const
{
	return QVariant(bool_value());
}

bool BoolProperty::bool_value() const
{
	bool bool_value;
	configurable_->get_config<bool>(config_key_, bool_value);
	return bool_value;
}

QString BoolProperty::to_string(bool value) const
{
	return value ? QString("true") : QString("false");
}

QString BoolProperty::to_string(const QVariant &qvar) const
{
	return this->to_string(qvar.toBool());
}

QString BoolProperty::to_string() const
{
	return this->to_string(bool_value());
}

bool BoolProperty::list_config()
{
	// No list for boolean properties!
	return false;
}

void BoolProperty::change_value(const QVariant &qvar)
{
	configurable_->set_config(config_key_, qvar.toBool());
	Q_EMIT value_changed(qvar);
}

void BoolProperty::on_value_changed(Glib::VariantBase gvar)
{
	Q_EMIT value_changed(QVariant(g_variant_get_boolean(gvar.gobj())));
}

} // namespace properties
} // namespace data
} // namespace sv
