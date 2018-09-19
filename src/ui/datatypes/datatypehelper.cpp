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

#include <memory>

#include "datatypehelper.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/properties/baseproperty.hpp"
#include "src/ui/datatypes/boolcheckbox.hpp"
#include "src/ui/datatypes/doublespinbox.hpp"
#include "src/ui/datatypes/int32spinbox.hpp"
#include "src/ui/datatypes/measuredquantitycombobox.hpp"
#include "src/ui/datatypes/stringcombobox.hpp"
#include "src/ui/datatypes/uint64spinbox.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace ui {
namespace datatypes {
namespace datatypehelper {

QWidget *get_widget_for_property(
	shared_ptr<sv::devices::properties::BaseProperty> property,
	bool auto_commit, bool auto_update)
{
	devices::DataType data_type =
		devices::deviceutil::get_data_type_for_config_key(
			property->config_key());

	switch (data_type) {
	case devices::DataType::Int32:
		return new Int32SpinBox(property, auto_commit, auto_update);
		break;
	case devices::DataType::UInt64:
		return new UInt64SpinBox(property, auto_commit, auto_update);
		break;
	case devices::DataType::Double:
		return new DoubleSpinBox(property, auto_commit, auto_update);
		break;
	case devices::DataType::String:
		return new StringComboBox(property, auto_commit, auto_update);
		break;
	case devices::DataType::Bool:
		return new BoolCheckBox(property, auto_commit, auto_update);
		break;
	case devices::DataType::MQ:
		return new MeasuredQuantityComboBox(property, auto_commit, auto_update);
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

} // namespace datatypehelper
} // namespace datatypes
} // namespace ui
} // namespace sv
