/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>
#include <utility>
#include <vector>

#include <QDebug>
#include <QString>
#include <QVariant>

#include "doublerangeproperty.hpp"
#include "src/util.hpp"
#include "src/devices/configurable.hpp"

using std::make_pair;
using std::shared_ptr;
using std::vector;

namespace sv {
namespace data {
namespace properties {

DoubleRangeProperty::DoubleRangeProperty(
		shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key)
{
	if (is_listable_)
		DoubleRangeProperty::list_config();
}

QVariant DoubleRangeProperty::value() const
{
	return QVariant::fromValue(double_range_value());
}

double_range_t DoubleRangeProperty::double_range_value() const
{
	Glib::VariantContainerBase gvar;
	configurable_->get_container_config(config_key_, gvar);

	size_t child_cnt = gvar.get_n_children();
	if (child_cnt != 2) {
		throw std::runtime_error(QString(
			"DoubleRangeProperty::double_range_value(): ").append(
			"container should have 2 child, but has %1").arg(child_cnt).
			toStdString());
	}

	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	double low =
		Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	double high =
		Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();

	return make_pair(low, high);
}

QString DoubleRangeProperty::to_string(data::double_range_t value) const
{
	QString str = QString("%1 - %2").arg(
		QString::number(value.first, 'f'), QString::number(value.second, 'f'));
	if (unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless)
		str.append(" ").append(datautil::format_unit(unit_));

	return str;
}

QString DoubleRangeProperty::to_string(const QVariant &qvar) const
{
	return this->to_string(qvar.value<data::double_range_t>());
}

QString DoubleRangeProperty::to_string() const
{
	return this->to_string(double_range_value());
}

vector<data::double_range_t> DoubleRangeProperty::list_values() const
{
	return values_list_;
}

bool DoubleRangeProperty::list_config()
{
	values_list_.clear();

	Glib::VariantContainerBase gvar;
	if (!configurable_->list_config(config_key_, gvar))
		return false;

	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		double low = Glib::VariantBase::cast_dynamic
			<Glib::Variant<double>>(gvar.get_child(0)).get();
		double high = Glib::VariantBase::cast_dynamic
			<Glib::Variant<double>>(gvar.get_child(1)).get();

		values_list_.push_back(make_pair(low, high));
	}

	Q_EMIT list_changed();

	return true;
}

/**
 * TODO: When glibmm >= 2.52 is more supported and tuple bug is fixed,
 *       use the template function with tuple<double, double>:
 *
 *       set_config(config_key_, std::tuple<double, double>);
 */
void DoubleRangeProperty::change_value(const QVariant &qvar)
{
	data::double_range_t range = qvar.value<data::double_range_t>();

	Glib::VariantBase gvar_low = Glib::Variant<double>::create(range.first);
	Glib::VariantBase gvar_high = Glib::Variant<double>::create(range.second);

	vector<Glib::VariantBase> gcontainer;
	gcontainer.push_back(gvar_low);
	gcontainer.push_back(gvar_high);

	configurable_->set_container_config(config_key_, gcontainer);
	Q_EMIT value_changed(qvar);
}

void DoubleRangeProperty::on_value_changed(Glib::VariantBase gvar)
{
	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	double low =
		Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	double high =
		Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();

	Q_EMIT value_changed(QVariant::fromValue(make_pair(low, high)));
}

} // namespace properties
} // namespace data
} // namespace sv
