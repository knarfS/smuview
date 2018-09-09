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

#include <string>

#include <QDebug>

#include "stringproperty.hpp"
#include "src/devices/configurable.hpp"

using std::string;

namespace sv {
namespace devices {
namespace properties {

StringProperty::StringProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key),
	list_values_(QStringList())
{
	if (is_listable_)
		configurable_->list_config_string_array(config_key_, list_values_);
}

QVariant StringProperty::value() const
{
	return QVariant(string_value());
}

QString StringProperty::string_value() const
{
	return QString("TODO"); //configurable_->get_config<string>(config_key_);
}

QStringList StringProperty::list_values() const
{
	return list_values_;
}

void StringProperty::change_value(const QVariant qvar)
{
	qvar.toString().toStdString();
	//configurable_->set_config(config_key_, qvar.toString().toStdString());
}

void StringProperty::on_value_changed(Glib::VariantBase g_var)
{
	Q_EMIT value_changed(QVariant(g_variant_get_string(g_var.gobj(), NULL)));
}

} // namespace datatypes
} // namespace devices
} // namespace sv
