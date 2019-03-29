/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <QWidget>
#include <qwt_plot_magnifier.h>

#include "plotmagnifier.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

PlotMagnifier::PlotMagnifier(QWidget *canvas) :
	QwtPlotMagnifier(canvas)
{
}

void PlotMagnifier::rescale(double factor)
{
	QwtPlotMagnifier::rescale(factor);
	Q_EMIT magnified(factor);
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
