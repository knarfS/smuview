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

#ifndef UI_WIDGETS_PLOT_PLOTMAGNIFIER_HPP
#define UI_WIDGETS_PLOT_PLOTMAGNIFIER_HPP

#include <QWidget>
#include <qwt_plot_magnifier.h>

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

/**
 * QwtPlotMagnifier doesn't have signals, so we have to create them.
 *
 * TODO: Reimplement to zomm in/out at the mouse pointer position, not only
 * at the center of the canvas.
 */
class PlotMagnifier : public QwtPlotMagnifier
{
    Q_OBJECT

public:
	PlotMagnifier(QWidget *canvas);

protected:
	void rescale(double factor) override;

Q_SIGNALS:
	void magnified(double factor);

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_PLOTMAGNIFIER_HPP
