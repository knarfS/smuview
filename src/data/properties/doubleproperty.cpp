/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2022 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QString>
#include <QVariant>

#include "doubleproperty.hpp"
#include "src/util.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace data {
namespace properties {

DoubleProperty::DoubleProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key),
	min_(std::numeric_limits<double>::lowest()),
	max_(std::numeric_limits<double>::max()),
	step_(0.001), //std::numeric_limits<double>::epsilon()
	total_digits_(data::DefaultTotalDigits),
	decimal_places_(data::DefaultDecimalPlaces)
{
	if (is_listable_)
		DoubleProperty::list_config();
}

QVariant DoubleProperty::value() const
{
	return QVariant(double_value());
}

double DoubleProperty::double_value() const
{
	double double_value;
	configurable_->get_config<double>(config_key_, double_value);
	return double_value;
}

QString DoubleProperty::to_string(double value) const
{
	QString str_val("");
	QString si_prefix("");
	util::format_value_si_autoscale(value, total_digits_, decimal_places_,
		str_val, si_prefix);
	QString unit_str = datautil::format_unit(unit_);
	if (!si_prefix.isEmpty() || !unit_str.isEmpty())
		str_val.append(" ").append(si_prefix).append(unit_str);

	return str_val;
}

QString DoubleProperty::to_string(const QVariant &qvar) const
{
	return this->to_string(qvar.toDouble());
}

QString DoubleProperty::to_string() const
{
	return this->to_string(double_value());
}

double DoubleProperty::min() const
{
	return min_;
}

double DoubleProperty::max() const
{
	return max_;
}

double DoubleProperty::step() const
{
	return step_;
}

int DoubleProperty::total_digits() const
{
	return total_digits_;
}

int DoubleProperty::decimal_places() const
{
	return decimal_places_;
}

bool DoubleProperty::list_config()
{
	Glib::VariantContainerBase gvar;
	if (!configurable_->list_config(config_key_, gvar))
		return false;

	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	min_ = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	max_ = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	step_ = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();

	total_digits_ = util::count_double_digits(max_, step_);
	decimal_places_ = util::count_decimal_places(step_);

	Q_EMIT list_changed();

	return true;
}

void DoubleProperty::change_value(const QVariant &qvar)
{
	configurable_->set_config(config_key_, qvar.toDouble());
	Q_EMIT value_changed(qvar);
}

void DoubleProperty::on_value_changed(Glib::VariantBase gvar)
{
	Q_EMIT value_changed(QVariant(g_variant_get_double(gvar.gobj())));
}

} // namespace properties
} // namespace data
} // namespace sv
