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

#ifndef VIEWS_DEMODMMCONTROLVIEW_HPP
#define VIEWS_DEMODMMCONTROLVIEW_HPP

#include <memory>
#include <tuple>

#include <QPushButton>
#include <QString>

#include "src/views/baseview.hpp"

using std::shared_ptr;
using std::tuple;

namespace sigrok {
class Quantity;
class QuantityFlag;
}

namespace sv {

class Session;

namespace devices {
class Configurable;
}

namespace widgets {
class QuantityComboBox;
class QuantityFlagsList;
class ValueControl;
}

namespace views {

class DemoDMMControlView : public BaseView
{
	Q_OBJECT

public:
	DemoDMMControlView(const Session& session,
		shared_ptr<devices::Configurable> configurable,
		QWidget* parent = nullptr);

	QString title() const;

private:
	shared_ptr<devices::Configurable> configurable_;

	widgets::QuantityComboBox *quantity_box_;
	widgets::QuantityFlagsList *quantity_flags_list_;
	QPushButton *set_button_;
	widgets::ValueControl *amplitude_control_;
	widgets::ValueControl *offset_control_;

	void setup_ui();
	void connect_signals();
	void init_values();

private Q_SLOTS:
	void on_quantity_set();
	void on_amplitude_changed(const double);
	void on_offset_changed(const double);

};

} // namespace views
} // namespace sv

#endif // VIEWS_DEMODMMCONTROLVIEW_HPP

