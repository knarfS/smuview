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

#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

#include <glib.h>

#include <QDebug>
#include <QString>
#include <QVariant>

#include "measuredquantityproperty.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/configurable.hpp"

using std::make_pair;
using std::set;
using std::vector;

Q_DECLARE_METATYPE(sv::data::measured_quantity_t)

namespace sv {
namespace data {
namespace properties {

MeasuredQuantityProperty::MeasuredQuantityProperty(
		shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key)
{
	if (is_listable_)
		MeasuredQuantityProperty::list_config();
}

QVariant MeasuredQuantityProperty::value() const
{
	return QVariant().fromValue(measured_quantity_value());
}

/**
 * TODO: When glibmm >= 2.52 is more supported and tuple bug is fixed,
 *       use the template function and return tuple<uint32_t, uint64_t>:
 *
 *       return get_config<std::tuple<uint32_t, uint64_t>>(sigrok::ConfigKey);
 */
data::measured_quantity_t
MeasuredQuantityProperty::measured_quantity_value() const
{
	Glib::VariantContainerBase gvar =
		configurable_->get_container_config(config_key_);

	size_t child_cnt = gvar.get_n_children();
	if (child_cnt != 2)
		throw std::runtime_error(QString(
			"MeasuredQuantityProperty::measured_quantity_value(): ").append(
			"container (mq) should have 2 child, but has %1").arg(child_cnt).
			toStdString());

	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	uint32_t sr_q =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint32_t>>(gvar).get();
	data::Quantity quantity = data::datautil::get_quantity(sr_q);

	iter.next_value(gvar);
	uint64_t sr_qflags =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(gvar).get();
	set<data::QuantityFlag> quantity_flags =
		data::datautil::get_quantity_flags(sr_qflags);

	return make_pair(quantity, quantity_flags);
}

QString MeasuredQuantityProperty::to_string(data::measured_quantity_t value) const
{
	return data::datautil::format_measured_quantity(value);
}

QString MeasuredQuantityProperty::to_string(const QVariant qvar) const
{
	return this->to_string(qvar.value<data::measured_quantity_t>());
}

QString MeasuredQuantityProperty::to_string() const
{
	return this->to_string(measured_quantity_value());
}

vector<data::measured_quantity_t> MeasuredQuantityProperty::list_values() const
{
	return measured_quantity_list_;
}

bool MeasuredQuantityProperty::list_config()
{
	measured_quantity_list_.clear();

	Glib::VariantContainerBase gvar;
	if (!configurable_->list_config(config_key_, gvar))
		return false;

	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		uint32_t sr_q = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint32_t>>(gvar.get_child(0)).get();
		data::Quantity quantity = data::datautil::get_quantity(sr_q);
		uint64_t sr_qflags = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint64_t>>(gvar.get_child(1)).get();
		set<data::QuantityFlag> quantity_flags =
			data::datautil::get_quantity_flags(sr_qflags);

		measured_quantity_list_.push_back(make_pair(quantity, quantity_flags));
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
void MeasuredQuantityProperty::change_value(const QVariant qvar)
{
	data::measured_quantity_t mq = qvar.value<data::measured_quantity_t>();

	uint32_t sr_q_id = data::datautil::get_sr_quantity_id(mq.first);
	Glib::VariantBase gvar_q = Glib::Variant<uint32_t>::create(sr_q_id);
	uint64_t sr_qfs_id = data::datautil::get_sr_quantity_flags_id(mq.second);
	Glib::VariantBase gvar_qfs = Glib::Variant<uint64_t>::create(sr_qfs_id);

	vector<Glib::VariantBase> gcontainer;
	gcontainer.push_back(gvar_q);
	gcontainer.push_back(gvar_qfs);

	configurable_->set_container_config(config_key_, gcontainer);
	Q_EMIT value_changed(qvar);
}

void MeasuredQuantityProperty::on_value_changed(Glib::VariantBase g_var)
{
	Q_EMIT value_changed(QVariant(g_variant_get_string(g_var.gobj(), NULL)));
}

} // namespace properties
} // namespace data
} // namespace sv
