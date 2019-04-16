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

#ifndef UI_DATATYPES_MEASUREDQUANTITYCOMBOBOX_HPP
#define UI_DATATYPES_MEASUREDQUANTITYCOMBOBOX_HPP

#include <memory>

#include <QComboBox>
#include <QVariant>

#include "src/ui/datatypes/basewidget.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
namespace properties {
class BaseProperty;
}
}

namespace ui {
namespace datatypes {

class MeasuredQuantityComboBox : public QComboBox, public BaseWidget
{
	Q_OBJECT

public:
	MeasuredQuantityComboBox(
		shared_ptr<sv::devices::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QWidget *parent = nullptr);

	QVariant variant_value() const override;

private:
	void setup_ui();
	void connect_signals();
	void connect_widget_2_prop_signals();
	void disconnect_widget_2_prop_signals();

private Q_SLOTS:
	/**
	 * Signal handling for Widget -> Property
	 */
	void value_changed(int);
	/**
	 * Signal handling for Property -> Widget
	 */
	void on_value_changed(const QVariant);

};

} // namespace datatypes
} // namespace ui
} // namespece sv

#endif // UI_DATATYPES_MEASUREDQUANTITYCOMBOBOX_HPP

