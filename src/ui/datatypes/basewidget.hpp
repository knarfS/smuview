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

#ifndef UI_DATATYPES_BASEWIDEGT_HPP
#define UI_DATATYPES_BASEWIDEGT_HPP

#include <memory>

#include <QVariant>

using std::shared_ptr;

namespace sv {

namespace devices {
namespace properties {
class BaseProperty;
}
}

namespace ui {
namespace datatypes {

class BaseWidget
{

public:
	BaseWidget(
		shared_ptr<devices::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update);

	virtual QVariant variant_value() const = 0;

protected:
	const bool auto_commit_;
	const bool auto_update_;
	shared_ptr<devices::properties::BaseProperty> property_;

protected: // Q_SLOTS
	/**
	 * Signal handling for Widget -> Property
	virtual void value_changed(const double) = 0;
	 */
	/**
	 * Signal handling for Property -> Widget
	virtual void on_value_changed(const QVariant) = 0;
	 */

};

} // namespace datatypes
} // namespace ui
} // namespece sv

#endif // UI_DATATYPES_BASEWIDEGT_HPP
