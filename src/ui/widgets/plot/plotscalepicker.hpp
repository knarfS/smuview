/*
 * This file is part of the SmuView project.
 * This file is based on the QWT EventFilter Example.
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

#ifndef UI_WIDGETS_PLOT_PLOTSCALEPICKER_HPP
#define UI_WIDGETS_PLOT_PLOTSCALEPICKER_HPP

#include <QEvent>
#include <QObject>

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

class Plot;

class PlotScalePicker : public QObject
{
	Q_OBJECT

public:
	explicit PlotScalePicker(Plot *plot);

	virtual bool eventFilter(QObject *object, QEvent *event);

private:
	Plot *plot_;
	bool is_double_clicked;
	int last_pan_p_value_;
	double wheel_factor_;

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_PLOTSCALEPICKER_HPP
