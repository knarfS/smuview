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

#include <QDebug>
#include <QString>
#include <QVariant>

#include "doubleproperty.hpp"
#include "src/util.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace devices {
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
		if (list_config()) {
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
	return configurable_->get_config<double>(config_key_);
}

QString DoubleProperty::to_string() const
{
	// TODO: digits_, decimal_places_
	return QString::number(double_value(), 'f');
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

	return true;
}

void DoubleProperty::change_value(const QVariant qvar)
{
	configurable_->set_config(config_key_, qvar.toDouble());
	Q_EMIT value_changed(qvar);
}

void DoubleProperty::on_value_changed(Glib::VariantBase g_var)
{
	Q_EMIT value_changed(QVariant(g_variant_get_double(g_var.gobj())));
}

} // namespace datatypes
} // namespace devices
} // namespace sv
