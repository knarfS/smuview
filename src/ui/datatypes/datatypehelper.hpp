/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DATATYPES_DATATYPEHELPER_HPP
#define UI_DATATYPES_DATATYPEHELPER_HPP

#include <memory>

#include <QWidget>

using std::shared_ptr;

namespace sv {

namespace data {
namespace properties {
class BaseProperty;
}
}

namespace ui {
namespace datatypes {
namespace datatypehelper {

/**
 * Returns the generic datatype widget for the given property type.
 *
 * @param[in] property The Property
 * @param[in] auto_commit Widget is making auto commits
 * @param[in] auto_update Widget is auto updated
 *
 * @return The generic widget for the property type
 */
QWidget *get_widget_for_property(
	shared_ptr<sv::data::properties::BaseProperty>property,
	bool auto_commit, bool auto_update);

} // namespace datatypehelper
} // namespace datatypes
} // namespace ui
} // namespace sv

#endif // UI_DATATYPES_DATATYPEHELPER_HPP
