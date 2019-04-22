/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <QDebug>
#include <QString>
#include <QVariant>

#include "uint64rangeproperty.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/configurable.hpp"

using std::make_pair;
using std::string;
using std::vector;

Q_DECLARE_METATYPE(sv::data::uint64_range_t)

namespace sv {
namespace data {
namespace properties {

UInt64RangeProperty::UInt64RangeProperty(
		shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key)
{
	if (is_listable_)
		list_config();
}

QVariant UInt64RangeProperty::value() const
{
	return QVariant().fromValue(uint64_range_value());
}

/**
 * TODO: When glibmm >= 2.52 is more supported and tuple bug is fixed,
 *       use the template function and return tuple<uint64_t, uint64_t>:
 *
 *       return get_config<std::tuple<uint64_t, uint64_t>>(sigrok::ConfigKey);
 */
data::uint64_range_t UInt64RangeProperty::uint64_range_value() const
{
	Glib::VariantContainerBase gvar =
		configurable_->get_container_config(config_key_);

	size_t child_cnt = gvar.get_n_children();
	if (child_cnt != 2)
		throw std::runtime_error(QString(
			"UInt64RangeProperty::uint64_range_value(): ").append(
			"container should have 2 child, but has %1").arg(child_cnt).
			toStdString());

	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	uint64_t low =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(gvar).get();
	iter.next_value(gvar);
	uint64_t high =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(gvar).get();

	return make_pair(low, high);
}

QString UInt64RangeProperty::to_string(data::uint64_range_t value) const
{
	QString str = QString("%1 - %2").arg(value.first, value.second);
	if (unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless)
		str.append(" ").append(datautil::format_unit(unit_));

	return str;
}

QString UInt64RangeProperty::to_string(const QVariant qvar) const
{
	return this->to_string(qvar.value<data::uint64_range_t>());
}

QString UInt64RangeProperty::to_string() const
{
	return this->to_string(uint64_range_value());
}

vector<data::uint64_range_t> UInt64RangeProperty::list_values() const
{
	return values_list_;
}

bool UInt64RangeProperty::list_config()
{
	Glib::VariantContainerBase gvar;
	if (!configurable_->list_config(config_key_, gvar))
		return false;

	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		uint64_t low = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint64_t>>(gvar.get_child(0)).get();
		uint64_t high = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint64_t>>(gvar.get_child(1)).get();

		values_list_.push_back(make_pair(low, high));
	}

	return true;
}

/**
 * TODO: When glibmm >= 2.52 is more supported and tuple bug is fixed,
 *       use the template function with tuple<uint32_t, uint64_t>:
 *
 *       set_config(config_key_, std::tuple<uint32_t, uint64_t>);
 */
void UInt64RangeProperty::change_value(const QVariant qvar)
{
	data::uint64_range_t range = qvar.value<data::uint64_range_t>();

	Glib::VariantBase gvar_low = Glib::Variant<uint64_t>::create(range.first);
	Glib::VariantBase gvar_high = Glib::Variant<uint64_t>::create(range.second);

	vector<Glib::VariantBase> gcontainer;
	gcontainer.push_back(gvar_low);
	gcontainer.push_back(gvar_high);

	configurable_->set_container_config(config_key_, gcontainer);
	Q_EMIT value_changed(qvar);
}

void UInt64RangeProperty::on_value_changed(Glib::VariantBase g_var)
{
	Glib::VariantIter iter(g_var);
	iter.next_value(g_var);
	uint64_t low =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(g_var).get();
	iter.next_value(g_var);
	uint64_t high =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(g_var).get();

	Q_EMIT value_changed(QVariant().fromValue(make_pair(low, high)));
}

} // namespace properties
} // namespace data
} // namespace sv
