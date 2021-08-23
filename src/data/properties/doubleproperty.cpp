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

#include <limits>

#include <QDebug>
#include <QString>
#include <QVariant>

#include "doubleproperty.hpp"
#include "src/util.hpp"
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
	decimal_places_(3)
{
	if (is_listable_) {
		if (DoubleProperty::list_config()) {
			digits_ = util::count_double_digits(max_, step_);
			decimal_places_ = util::get_decimal_places(step_);
		}
	}
}

QVariant DoubleProperty::value() const
{
	return QVariant(double_value());
}

double DoubleProperty::double_value() const
{
	double value;
	configurable_->get_config<double>(config_key_, value);
	return value;
}

QString DoubleProperty::to_string(double value) const
{
	QString str = QString("%1").arg(value, digits_, 'f', decimal_places_);
	if (unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless)
		str.append(" ").append(datautil::format_unit(unit_));

	return str;
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

uint DoubleProperty::digits() const
{
	return digits_;
}

uint DoubleProperty::decimal_places() const
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
