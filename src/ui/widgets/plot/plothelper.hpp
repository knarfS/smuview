/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_PLOT_PLOTHELPER_HPP
#define UI_WIDGETS_PLOT_PLOTHELPER_HPP

#include <map>

#include <QString>

#include <qwt_plot.h>

using std::map;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {
namespace plothelper {

typedef map<QwtPlot::Axis, QString> axis_name_map_t;

namespace {

axis_name_map_t axis_name_map = {
	{ QwtPlot::yLeft, QString("Left") },
	{ QwtPlot::yRight, QString("Right") },
	{ QwtPlot::xBottom, QString("Bottom") },
	{ QwtPlot::xTop, QString("Top") },
};

}

/**
 * Format a QwtPlot::Axis to a string
 *
 * @param axis The QwtPlot::Axis to format.
 *
 * @return The formatted axis.
 */
QString format_axis(QwtPlot::Axis axis);

} // namespace plothelper
} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_PLOTHELPER_HPP
