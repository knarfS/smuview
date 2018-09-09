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

#include "int32property.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace devices {
namespace properties {

Int32Property::Int32Property(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key)
{
	if (is_listable_) {
		// TODO: Add templeate for list_config_min_max_step<int32_t>
		//configurable_->list_config_min_max_step(config_key_, min_, max_, step_);
		min_ = std::numeric_limits<int32_t>::lowest();
		max_ = std::numeric_limits<int32_t>::max();
		step_ = 1;
	}
	else {
		min_ = std::numeric_limits<int32_t>::lowest();
		max_ = std::numeric_limits<int32_t>::max();
		step_ = 1;
	}

	/*
	if (unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless) {
		this->setSuffix(QString(" %1").arg(data::datautil::format_unit(unit_)));
	}
	*/
}

QVariant Int32Property::value() const
{
	return QVariant(int32_value());
}

int32_t Int32Property::int32_value() const
{
	return configurable_->get_config<int32_t>(config_key_);
}

int32_t Int32Property::min() const
{
	return min_;
}

int32_t Int32Property::max() const
{
	return max_;
}

int32_t Int32Property::step() const
{
	return step_;
}

void Int32Property::change_value(const QVariant qvar)
{
	configurable_->set_config(config_key_, qvar.toInt());
}

void Int32Property::on_value_changed(Glib::VariantBase g_var)
{
	Q_EMIT value_changed(QVariant(g_variant_get_int32(g_var.gobj())));
}

} // namespace datatypes
} // namespace devices
} // namespace sv
