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

#include <QResizeEvent>
#include <qwt_painter.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>

#include "plotcanvas.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

PlotCanvas::PlotCanvas(QwtPlot *plot) : QwtPlotCanvas(plot)
{
	/*
	 * NOTE:
	 * The backing store is important, when working with widget overlays
	 * (f.e rubberbands for zooming). Here we don't have them and the
	 * internal backing store of QWidget is good enough.
	 */
	setPaintAttribute(QwtPlotCanvas::BackingStore, false);
	/*
	 * NOTE:
	 * ImmediatePaint is necessary so "old" curves will be deleted.
	 * QwtPlot::repaint() in replot() will also work
	 */
	setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);
	setBorderRadius(10);

	if (QwtPainter::isX11GraphicsSystem()) {
		/*
		 * NOTE:
		 * Disabling the backing store of Qt improves the performance for
		 * the direct painter even more, but the canvas becomes a native
		 * window of the window system, receiving paint events for resize
		 * and expose operations. Those might be expensive when there are
		 * many points and the backing store of the canvas is disabled. So
		 * in this application we better don't disable both backing stores.
		 */
		if (testPaintAttribute(QwtPlotCanvas::BackingStore)) {
			setAttribute(Qt::WA_PaintOnScreen, true);
			setAttribute(Qt::WA_NoSystemBackground, true);
		}
	}

	setupPalette();
}

void PlotCanvas::resizeEvent(QResizeEvent *event)
{
	Q_EMIT size_changed();
	QwtPlotCanvas::resizeEvent(event);
}

void PlotCanvas::setupPalette()
{
	QPalette pal = palette();

	QLinearGradient gradient;
	gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
	gradient.setColorAt(0.0, QColor(0, 49, 110));
	gradient.setColorAt(1.0, QColor(0, 87, 174));

	pal.setBrush(QPalette::Window, QBrush(gradient));

	// QPalette::WindowText is used for the curve color
	pal.setColor(QPalette::WindowText, Qt::green);

	setPalette(pal);
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
