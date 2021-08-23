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
	return QVariant::fromValue(measured_quantity_value());
}

data::measured_quantity_t
MeasuredQuantityProperty::measured_quantity_value() const
{
	data::measured_quantity_t value;
	configurable_->get_measured_quantity_config(config_key_, value);
	return value;
}

QString MeasuredQuantityProperty::to_string(
	const data::measured_quantity_t &value) const
{
	return data::datautil::format_measured_quantity(value);
}

QString MeasuredQuantityProperty::to_string(const QVariant &qvar) const
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

void MeasuredQuantityProperty::change_value(const QVariant &qvar)
{
	data::measured_quantity_t mq = qvar.value<data::measured_quantity_t>();
	configurable_->set_measured_quantity_config(config_key_, mq);
	Q_EMIT value_changed(qvar);
}

void MeasuredQuantityProperty::on_value_changed(Glib::VariantBase gvar)
{
	Q_EMIT value_changed(QVariant(g_variant_get_string(gvar.gobj(), nullptr)));
}

} // namespace properties
} // namespace data
} // namespace sv
