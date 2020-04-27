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

#include <string>

#include <glibmm.h>

#include <QDebug>
#include <QString>
#include <QVariant>

#include "stringproperty.hpp"
#include "src/devices/configurable.hpp"

using std::string;

namespace sv {
namespace data {
namespace properties {

StringProperty::StringProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key),
	string_list_(QStringList())
{
	if (is_listable_) {
		if (!StringProperty::list_config())
			is_listable_ = false;
	}
}

QVariant StringProperty::value() const
{
	return QVariant(string_value());
}

QString StringProperty::string_value() const
{
	return QString::fromStdString(
		configurable_->get_config<string>(config_key_));
}

QString StringProperty::to_string(const QVariant qvar) const
{
	return qvar.toString();
}

QString StringProperty::to_string() const
{
	return string_value();
}

QStringList StringProperty::list_values() const
{
	return string_list_;
}

bool StringProperty::list_config()
{
	string_list_.clear();

	Glib::VariantContainerBase gvar;
	if (!configurable_->list_config(config_key_, gvar))
		return false;

	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		string_list_.append(QString::fromStdString(
			Glib::VariantBase::cast_dynamic<Glib::Variant<string>>(gvar).get()));
	}

	Q_EMIT list_changed();

	return true;
}

void StringProperty::change_value(const QVariant qvar)
{
	// We have to use Glib::ustring here, to get a variant type of 's'.
	// std::string will create a variant type of 'ay'
	configurable_->set_config<Glib::ustring>(
		config_key_, Glib::ustring(qvar.toString().toStdString()));
	Q_EMIT value_changed(qvar);
}

void StringProperty::on_value_changed(Glib::VariantBase gvar)
{
	Q_EMIT value_changed(QVariant(g_variant_get_string(gvar.gobj(), NULL)));
}

} // namespace datatypes
} // namespace data
} // namespace sv
