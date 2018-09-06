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

#ifndef UI_DATATYPES_DATATYPEHELPER_HPP
#define UI_DATATYPES_DATATYPEHELPER_HPP

#include <memory>

#include <QWidget>

#include "src/data/datautil.hpp"
#include "src/devices/deviceutil.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
class Configurable;
}

namespace ui {
namespace datatypes {
namespace datatypehelper {

/**
 * Returns the generic datatype widget for the given config key.
 *
 * @param[in] configurable The Configurable
 * @param[in] config_key The ConfigKey
 * @param[in] unit The Unit
 * @param[in] auto_commit Auto commit when widget is changed
 *
 * @return The generic widget for the config key
 */
QWidget *get_widget_for_config_key(shared_ptr<devices::Configurable>,
	devices::ConfigKey, data::Unit, bool);

} // namespace datatypehelper
} // namespace datatypes
} // namespace ui
} // namespace sv

#endif // UI_DATATYPES_DATATYPEHELPER_HPP
