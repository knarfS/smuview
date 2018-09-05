/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>

#include <QDebug>

#include "setvalueblock.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"

namespace sv {
namespace processing {

SetValueBlock::SetValueBlock() : BaseBlock()
{
}

void SetValueBlock::init()
{
}

void SetValueBlock::run()
{
	assert(configurable_);
	assert(config_key_);

	qWarning() << "SetValueBlock: value = " << value_;

	configurable_->set_config(config_key_, value_);
	configurable_->config_changed(config_key_, value_);
}

void SetValueBlock::set_configurable(shared_ptr<devices::Configurable> configurable)
{
	configurable_ = configurable;
}

void SetValueBlock::set_config_key(devices::ConfigKey key)
{
	config_key_ = key;
}

void SetValueBlock::set_value(double value)
{
	value_ = value;
}

} // namespace processing
} // namespace sv
