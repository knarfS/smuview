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

#include "datatypehelper.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/datatypes/boolcheckbox.hpp"
#include "src/ui/datatypes/floatspinbox.hpp"
#include "src/ui/datatypes/int32spinbox.hpp"

namespace sv {
namespace ui {
namespace datatypes {
namespace datatypehelper {

QWidget *get_widget_for_config_key(
	shared_ptr<devices::Configurable> configurable,
	devices::ConfigKey config_key, data::Unit unit, bool auto_commit)
{
	devices::DataType data_type =
		devices::deviceutil::get_data_type_for_config_key(config_key);

	switch (data_type) {
	case devices::DataType::Int32:
		return new Int32SpinBox(configurable, config_key, unit, auto_commit);
		break;
	case devices::DataType::Float:
		return new FloatSpinBox(configurable, config_key, unit, auto_commit);
		break;
	case devices::DataType::Bool:
		return new BoolCheckBox(configurable, config_key, auto_commit);
		break;
	default:
		return NULL;
		break;
	}
}

} // namespace datatypehelper
} // namespace datatypes
} // namespace ui
} // namespace sv
