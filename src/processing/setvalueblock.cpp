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
#include "src/devices/properties/baseproperty.hpp"
#include "src/processing/processor.hpp"

using std::shared_ptr;

namespace sv {
namespace processing {

SetValueBlock::SetValueBlock(const shared_ptr<Processor> processor) :
	BaseBlock(processor)
{
}

void SetValueBlock::init()
{
}

void SetValueBlock::run()
{
	assert(property_);
	assert(value_);

	//qWarning() << "SetValueBlock: value = " << value_;

	property_->change_value(value_);
}

shared_ptr<devices::properties::BaseProperty> SetValueBlock::property() const
{
	return property_;
}

void SetValueBlock::set_property(
	shared_ptr<devices::properties::BaseProperty> property)
{
	property_ = property;
}

QVariant SetValueBlock::value() const
{
	return value_;
}

void SetValueBlock::set_value(QVariant value)
{
	value_ = value;
}

} // namespace processing
} // namespace sv
