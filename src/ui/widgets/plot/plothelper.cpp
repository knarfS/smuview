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

#include <map>

#include <QString>

#include <qwt_plot.h>

#include "plothelper.hpp"

using std::map;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {
namespace plothelper {

QString format_axis(QwtPlot::Axis axis)
{
	return axis_name_map[axis];
}

} // namespace plothelper
} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
