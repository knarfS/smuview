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

#include "uint64property.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace devices {
namespace properties {

UInt64Property::UInt64Property(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key)
{
	if (is_listable_) {
		// TODO: Add templeate for list_config_min_max_step<uint64_t>
		//configurable_->list_config_min_max_step(config_key_, min_, max_, step_);
		min_ = std::numeric_limits<uint64_t>::lowest();
		max_ = std::numeric_limits<uint64_t>::max();
		step_ = 1;
	}
	else {
		min_ = std::numeric_limits<uint64_t>::lowest();
		max_ = std::numeric_limits<uint64_t>::max();
		step_ = 1;
	}

	/*
	if (unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless) {
		this->setSuffix(QString(" %1").arg(data::datautil::format_unit(unit_)));
	}
	*/
}

QVariant UInt64Property::value() const
{
	return QVariant((qulonglong)uint64_value());
}

uint64_t UInt64Property::uint64_value() const
{
	return configurable_->get_config<uint64_t>(config_key_);
}

uint64_t UInt64Property::min() const
{
	return min_;
}

uint64_t UInt64Property::max() const
{
	return max_;
}

uint64_t UInt64Property::step() const
{
	return step_;
}

void UInt64Property::change_value(const QVariant qvar)
{
	configurable_->set_config(config_key_, (uint64_t)qvar.toULongLong());
}

void UInt64Property::on_value_changed(Glib::VariantBase g_var)
{
	Q_EMIT value_changed(QVariant(
		(qulonglong)g_variant_get_uint64(g_var.gobj())));
}

} // namespace datatypes
} // namespace devices
} // namespace sv
