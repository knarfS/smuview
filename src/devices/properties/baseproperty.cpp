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

#include <QDebug>

#include "baseproperty.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace devices {
namespace properties {

BaseProperty::BaseProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	configurable_(configurable),
	config_key_(config_key)
{
	data_type_ = deviceutil::get_data_type_for_config_key(config_key_);
	unit_ = deviceutil::get_unit_for_config_key(config_key_);
	is_getable_ = configurable_->has_get_config(config_key_);
	is_setable_ = configurable_->has_set_config(config_key_);
	is_listable_ = configurable_->has_list_config(config_key_);
}

shared_ptr<devices::Configurable> BaseProperty::configurable() const
{
	return configurable_;
}

devices::ConfigKey BaseProperty::config_key() const
{
	return config_key_;
}

devices::DataType BaseProperty::data_type() const
{
	return data_type_;
}

data::Unit BaseProperty::unit() const
{
	return unit_;
}

bool BaseProperty::is_getable() const
{
	return is_getable_;
}

bool BaseProperty::is_setable() const
{
	return is_setable_;
}

bool BaseProperty::is_listable() const
{
	return is_listable_;
}

} // namespace properties
} // namespace devices
} // namespace sv
