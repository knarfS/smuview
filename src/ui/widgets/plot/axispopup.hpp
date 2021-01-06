/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_PLOT_AXISPOPUP_HPP
#define UI_WIDGETS_PLOT_AXISPOPUP_HPP

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QShowEvent>
#include <QWidget>

#include "src/ui/widgets/popup.hpp"
#include "src/ui/widgets/plot/plot.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

class AxisPopup : public widgets::Popup
{
	Q_OBJECT

public:
	AxisPopup(Plot *plot, int axis_id, QWidget *parent);

private:
	Plot *plot_;
	int axis_id_;

	QLineEdit *axis_lower_edit_;
	QCheckBox *axis_lower_locked_check_;
	QLineEdit *axis_upper_edit_;
	QCheckBox *axis_upper_locked_check_;
	QCheckBox *axis_log_check_;
	QDialogButtonBox *button_box_;

	void setup_ui();
	void showEvent(QShowEvent *event) override;

private Q_SLOTS:
	void on_accept();

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_AXISPOPUP_HPP
