/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_DEMOCONTROLVIEW_HPP
#define UI_VIEWS_DEMOCONTROLVIEW_HPP

#include <memory>

#include <QPushButton>
#include <QString>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace devices {
class Configurable;
}

namespace ui {

namespace data {
class QuantityComboBox;
class QuantityFlagsList;
}
namespace datatypes {
class DoubleControl;
class StringComboBox;
}

namespace views {

class DemoControlView : public BaseView
{
	Q_OBJECT

public:
	DemoControlView(Session& session,
		shared_ptr<sv::devices::Configurable> configurable,
		QWidget* parent = nullptr);

	QString title() const override;

private:
	shared_ptr<sv::devices::Configurable> configurable_;

	ui::data::QuantityComboBox *quantity_box_;
	ui::data::QuantityFlagsList *quantity_flags_list_;
	QPushButton *set_button_;
	ui::datatypes::StringComboBox *pattern_box_;
	ui::datatypes::DoubleControl *amplitude_control_;
	ui::datatypes::DoubleControl *offset_control_;

	void setup_ui();
	void connect_signals();

private Q_SLOTS:
	void on_quantity_set();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_DEMOCONTROLVIEW_HPP
