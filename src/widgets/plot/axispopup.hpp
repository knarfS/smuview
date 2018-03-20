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

#ifndef WIDGETS_PLOT_AXISPOPUP_HPP
#define WIDGETS_PLOT_AXISPOPUP_HPP

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QShowEvent>
#include <QWidget>

#include <src/widgets/popup.hpp>
#include "src/widgets/plot/plot.hpp"

namespace sv {
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

	QLineEdit *axis_min_edit_;
	QLineEdit *axis_max_edit_;
	QCheckBox *axis_log_check_;
	QDialogButtonBox *button_box_;

	void setup_ui();
	void showEvent(QShowEvent *event);

private Q_SLOTS:
	void on_accept();

};

} // namespace plot
} // namespace widgets
} // namespace sv

#endif // WIDGETS_PLOT_AXISPOPUP_HPP
