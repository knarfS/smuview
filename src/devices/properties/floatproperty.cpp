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

#include <limits>

#include <QDebug>

#include "floatproperty.hpp"
#include "src/util.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace devices {
namespace properties {

FloatProperty::FloatProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key),
	min_(std::numeric_limits<double>::lowest()),
	max_(std::numeric_limits<double>::max()),
	step_(0.001), //std::numeric_limits<double>::epsilon()
	decimal_places_(3)
{
	if (is_listable_) {
		if (list_config())
			decimal_places_ = util::get_decimal_places(step_);
	}

	/*
	if (unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless) {
		this->setSuffix(QString(" %1").arg(data::datautil::format_unit(unit_)));
	}
	*/
}

QVariant FloatProperty::value() const
{
	return QVariant(float_value());
}

double FloatProperty::float_value() const
{
	return configurable_->get_config<double>(config_key_);
}

double FloatProperty::min() const
{
	return min_;
}

double FloatProperty::max() const
{
	return max_;
}

double FloatProperty::step() const
{
	return step_;
}

int FloatProperty::decimal_places() const
{
	return decimal_places_;
}

bool FloatProperty::list_config()
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

void FloatProperty::change_value(const QVariant qvar)
{
	configurable_->set_config(config_key_, qvar.toDouble());
}

void FloatProperty::on_value_changed(Glib::VariantBase g_var)
{
	Q_EMIT value_changed(QVariant(g_variant_get_double(g_var.gobj())));
}

} // namespace datatypes
} // namespace devices
} // namespace sv
