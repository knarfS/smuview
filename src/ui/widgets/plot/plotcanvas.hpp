/*
 * This file is part of the SmuView project.
 * This file is based on the QWT Oscilloscope Example.
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


#ifndef UI_WIDGETS_PLOT_PLOTCANVAS_HPP
#define UI_WIDGETS_PLOT_PLOTCANVAS_HPP

#include <QResizeEvent>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

class PlotCanvas : public QwtPlotCanvas
{
	Q_OBJECT

public:
	explicit PlotCanvas(QwtPlot *plot = nullptr);

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	void setupPalette();

Q_SIGNALS:
	void size_changed();

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_PLOTCANVAS_HPP
