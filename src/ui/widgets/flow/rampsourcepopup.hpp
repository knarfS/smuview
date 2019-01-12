/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_FLOW_RAMPSOURCEPOPUP_HPP
#define UI_WIDGETS_FLOW_RAMPSOURCEPOPUP_HPP

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QShowEvent>
#include <QWidget>

#include <Node>

#include "src/ui/widgets/popup.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace flow {

class RampSourcePopup : public widgets::Popup
{
	Q_OBJECT

public:
	RampSourcePopup(QtNodes::Node &node, QWidget *parent);

private:
	QtNodes::Node &node_;

	QDoubleSpinBox *start_value_box_;
	QDoubleSpinBox *end_value_box_;
	QDoubleSpinBox *step_width_box_;
	QCheckBox *step_trigger_box_;
	QSpinBox *step_delay_box_;
	QDialogButtonBox *button_box_;

	void setup_ui();
	void showEvent(QShowEvent *event);

private Q_SLOTS:
	void on_accept();
	void on_step_trigger_changed();

};

} // namespace flow
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_FLOW_RAMPSOURCEPOPUP_HPP
