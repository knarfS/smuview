/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_MEASUREMENTCONTROLVIEW_HPP
#define UI_VIEWS_MEASUREMENTCONTROLVIEW_HPP

#include <memory>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace devices {
class Configurable;
}

namespace ui {

namespace datatypes {
class MeasuredQuantityComboBox;
class StringComboBox;
}

namespace views {

class MeasurementControlView : public BaseView
{
	Q_OBJECT

public:
	MeasurementControlView(Session& session,
		shared_ptr<sv::devices::Configurable> configurable,
		QWidget* parent = nullptr);

	QString title() const override;

private:
	shared_ptr<sv::devices::Configurable> configurable_;

	ui::datatypes::MeasuredQuantityComboBox *measured_quantity_box_;
	ui::datatypes::StringComboBox *range_box_;
	ui::datatypes::StringComboBox *digits_box_;

	void setup_ui();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_MEASUREMENTCONTROLVIEW_HPP
