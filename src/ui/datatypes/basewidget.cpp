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

#include "basewidget.hpp"
#include "src/devices/properties/baseproperty.hpp"

namespace sv {
namespace ui {
namespace datatypes {

BaseWidget::BaseWidget(
		shared_ptr<devices::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update) :
	auto_commit_(auto_commit),
	auto_update_(auto_update),
	property_(property)
{
}

} // namespace datatypes
} // namespace ui
} // namespace sv
