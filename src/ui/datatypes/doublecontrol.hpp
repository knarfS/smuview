/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DATATYPES_DOUBLECONTROL_HPP
#define UI_DATATYPES_DOUBLECONTROL_HPP

#include <memory>

#include <QGroupBox>

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

class DoubleDisplay;
class DoubleSlider;
class DoubleSpinBox;

class DoubleControl : public QGroupBox, public BaseWidget
{
	Q_OBJECT

public:
	DoubleControl(
		shared_ptr<sv::devices::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QString title, QWidget *parent = nullptr);

	QVariant variant_value() const override;

private:
	QString title_;

	DoubleSpinBox *spin_box_;
	DoubleSlider *slider_;
	DoubleDisplay *display_;

	void setup_ui();

};

} // namespace datatypes
} // namespace ui
} // namespace sv

#endif // UI_DATATYPES_DOUBLECONTROL_HPP
