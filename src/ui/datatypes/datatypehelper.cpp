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

#include <memory>

#include "datatypehelper.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/uint64property.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/datatypes/boolcheckbox.hpp"
#include "src/ui/datatypes/doublerangecombobox.hpp"
#include "src/ui/datatypes/doublespinbox.hpp"
#include "src/ui/datatypes/int32spinbox.hpp"
#include "src/ui/datatypes/measuredquantitycombobox.hpp"
#include "src/ui/datatypes/rationalcombobox.hpp"
#include "src/ui/datatypes/stringcombobox.hpp"
#include "src/ui/datatypes/uint64combobox.hpp"
#include "src/ui/datatypes/uint64rangecombobox.hpp"
#include "src/ui/datatypes/uint64spinbox.hpp"

using std::shared_ptr;
using std::static_pointer_cast;

namespace sv {
namespace ui {
namespace datatypes {
namespace datatypehelper {

QWidget *get_widget_for_property(
	shared_ptr<sv::data::properties::BaseProperty> property,
	bool auto_commit, bool auto_update)
{
	auto data_type = devices::deviceutil::get_data_type_for_config_key(
		property->config_key());

	switch (data_type) {
	case data::DataType::Int32:
		return new Int32SpinBox(property, auto_commit, auto_update);
		break;
	case data::DataType::UInt64:
	{
		// Special handling for different list types
		shared_ptr<data::properties::UInt64Property> uint64_prop =
			static_pointer_cast<data::properties::UInt64Property>(property);
		if (uint64_prop->list_values().size() > 0)
			return new UInt64ComboBox(property, auto_commit, auto_update);
		else
			return new UInt64SpinBox(property, auto_commit, auto_update);
		break;
	}
	case data::DataType::Double:
		return new DoubleSpinBox(property, auto_commit, auto_update);
		break;
	case data::DataType::RationalPeriod:
	case data::DataType::RationalVolt:
		return new RationalComboBox(property, auto_commit, auto_update);
		break;
	case data::DataType::String:
		return new StringComboBox(property, auto_commit, auto_update);
		break;
	case data::DataType::Bool:
		return new BoolCheckBox(property, auto_commit, auto_update);
		break;
	case data::DataType::MQ:
		return new MeasuredQuantityComboBox(property, auto_commit, auto_update);
		break;
	case data::DataType::DoubleRange:
		return new DoubleRangeComboBox(property, auto_commit, auto_update);
		break;
	case data::DataType::Uint64Range:
		return new UInt64RangeComboBox(property, auto_commit, auto_update);
		break;
	case data::DataType::KeyValue:
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
