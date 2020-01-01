/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include "rationalproperty.hpp"
#include "src/util.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/configurable.hpp"

using std::make_pair;
using std::string;
using std::vector;

Q_DECLARE_METATYPE(sv::data::rational_t)

namespace sv {
namespace data {
namespace properties {

RationalProperty::RationalProperty(
		shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key)
{
	if (is_listable_)
		list_config();
}

QVariant RationalProperty::value() const
{
	return QVariant().fromValue(rational_value());
}

/**
 * TODO: When glibmm >= 2.52 is more supported and tuple bug is fixed,
 *       use the template function and return tuple<uint64_t, uint64_t>:
 *
 *       return get_config<std::tuple<uint32_t, uint64_t>>(sigrok::ConfigKey);
 */
data::rational_t RationalProperty::rational_value() const
{
	Glib::VariantContainerBase gvar =
		configurable_->get_container_config(config_key_);

	size_t child_cnt = gvar.get_n_children();
	if (child_cnt != 2)
		throw std::runtime_error(QString(
			"RationalProperty::rational_value(): ").append(
			"container should have 2 child, but has %1").arg(child_cnt).
			toStdString());

	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	uint64_t p =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(gvar).get();
	iter.next_value(gvar);
	uint64_t q =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(gvar).get();

	return make_pair(p, q);
}

QString RationalProperty::to_string(data::rational_t value) const
{
	double d_val = value.first / (double)value.second;
	QString str_prefix("");
	QString str_val;
	util::format_value_si(d_val, -1, 0, str_val, str_prefix);
	str_val.append(" ").append(str_prefix).
		append(data::datautil::format_unit(unit_));

	return str_val;
}

QString RationalProperty::to_string(const QVariant qvar) const
{
	return this->to_string(qvar.value<data::rational_t>());
}

QString RationalProperty::to_string() const
{
	return this->to_string(rational_value());
}

vector<data::rational_t> RationalProperty::list_values() const
{
	return values_list_;
}

bool RationalProperty::list_config()
{
	values_list_.clear();

	Glib::VariantContainerBase gvar;
	if (!configurable_->list_config(config_key_, gvar))
		return false;

	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		uint64_t p = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint64_t>>(gvar.get_child(0)).get();
		uint64_t q = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint64_t>>(gvar.get_child(1)).get();

		values_list_.push_back(make_pair(p, q));
	}

	Q_EMIT list_changed();

	return true;
}

/**
 * TODO: When glibmm >= 2.52 is more supported and tuple bug is fixed,
 *       use the template function with tuple<uint32_t, uint64_t>:
 *
 *       set_config(config_key_, std::tuple<uint32_t, uint64_t>);
 */
void RationalProperty::change_value(const QVariant qvar)
{
	data::rational_t rational = qvar.value<data::rational_t>();

	Glib::VariantBase gvar_p = Glib::Variant<uint64_t>::create(rational.first);
	Glib::VariantBase gvar_q = Glib::Variant<uint64_t>::create(rational.second);

	vector<Glib::VariantBase> gcontainer;
	gcontainer.push_back(gvar_p);
	gcontainer.push_back(gvar_q);

	configurable_->set_container_config(config_key_, gcontainer);
	Q_EMIT value_changed(qvar);
}

void RationalProperty::on_value_changed(Glib::VariantBase g_var)
{
	Glib::VariantIter iter(g_var);
	iter.next_value(g_var);
	uint64_t p =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(g_var).get();
	iter.next_value(g_var);
	uint64_t q =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(g_var).get();

	Q_EMIT value_changed(QVariant().fromValue(make_pair(p, q)));
}

} // namespace datatypes
} // namespace data
} // namespace sv
