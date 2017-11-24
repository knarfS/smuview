/*
 * This file is part of the SmuView project.
 * This file is based on the QWT Oscilloscope Example.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QEvent>
#include <QRectF>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_curve_fitter.h>
#include <qwt_painter.h>
#include <qwt_legend.h>

#include "plot.hpp"
#include "src/data/basedata.hpp"
#include "src/data/curvedata.hpp"

namespace sv {
namespace widgets {

class Canvas: public QwtPlotCanvas
{
public:
	Canvas(QwtPlot *plot = NULL) :
		QwtPlotCanvas(plot)
	{
		// The backing store is important, when working with widget
		// overlays ( f.e rubberbands for zooming ).
		// Here we don't have them and the internal
		// backing store of QWidget is good enough.

		setPaintAttribute(QwtPlotCanvas::BackingStore, false);
		setBorderRadius(10);

		if (QwtPainter::isX11GraphicsSystem()) {
#if QT_VERSION < 0x050000
			// Even if not liked by the Qt development, Qt::WA_PaintOutsidePaintEvent
			// works on X11. This has a nice effect on the performance.

			setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
#endif

			// Disabling the backing store of Qt improves the performance
			// for the direct painter even more, but the canvas becomes
			// a native window of the window system, receiving paint events
			// for resize and expose operations. Those might be expensive
			// when there are many points and the backing store of
			// the canvas is disabled. So in this application
			// we better don't disable both backing stores.

			if (testPaintAttribute(QwtPlotCanvas::BackingStore)) {
				setAttribute(Qt::WA_PaintOnScreen, true);
				setAttribute(Qt::WA_NoSystemBackground, true);
			}
		}

		setupPalette();
	}

private:
	void setupPalette()
	{
		QPalette pal = palette();

#if QT_VERSION >= 0x040400
		QLinearGradient gradient;
		gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
		gradient.setColorAt(0.0, QColor(0, 49, 110));
		gradient.setColorAt(1.0, QColor(0, 87, 174));

		pal.setBrush(QPalette::Window, QBrush(gradient));
#else
		pal.setBrush(QPalette::Window, QBrush(color));
#endif

		// QPalette::WindowText is used for the curve color
		pal.setColor(QPalette::WindowText, Qt::green);

		setPalette(pal);
	}
};

Plot::Plot(data::CurveData *curve_data, QWidget *parent):
	QwtPlot(parent),
	curve_data_(curve_data),
	painted_points_(0),
	x_interval_(0.0, 30.0),
	y_interval_(0.0, 0.5),
	plot_interval_(200),
	timer_id_(-1)
{
	valueDirectPainter_ = new QwtPlotDirectPainter();

	setAutoReplot(false);
	setCanvas(new Canvas());

	plotLayout()->setAlignCanvasToScales(true);

	QwtLegend *legend = new QwtLegend;
	legend->setDefaultItemMode(QwtLegendData::Checkable);
	insertLegend(legend, QwtPlot::BottomLegend);

	// Time axis
	setAxisTitle(QwtPlot::xBottom, "Time [s]");
	//setAxisTitle(QwtPlot::xBottom, "Voltage [V]");
	//setAxisScale( QwtAxisId( QwtAxis::xBottom, 0 ), m_interval.minValue(), m_interval.maxValue() ); // TODO: Multiaxis
	setAxisScale(0, x_interval_.minValue(), x_interval_.maxValue());
	//setAxisAutoScale(0, true); // Not working!?

	//setAxesCount( QwtPlot::yLeft, 2 ); // TODO: Multiaxis
	// Current axis
	//QwtAxisId currentAxisId = QwtAxisId( QwtAxis::yLeft, 0 ); // TODO: Multiaxis
	int currentAxisId = QwtPlot::yLeft;
	//setAxisVisible( currentAxisId, true ); // TODO: Multiaxis
	setAxisTitle(currentAxisId, "Current [A]");
	setAxisScale(currentAxisId, y_interval_.minValue(), y_interval_.maxValue());
	//setAxisAutoScale(currentAxisId, false); // Not working!?

	// Voltage axis
	/*
	QwtAxisId voltageAxisId = QwtAxisId( QwtAxis::yLeft, 1 ); // TODO: Multiaxis
	setAxisVisible( voltageAxisId, true); // TODO: Multiaxis
	setAxisTitle( voltageAxisId, "Voltage [V]" );
	setAxisScale( voltageAxisId, 0.0, 5.0 );
	*/

	QwtPlotGrid *grid = new QwtPlotGrid();
	grid->setPen(Qt::gray, 0.0, Qt::DotLine);
	grid->enableX(true);
	grid->enableXMin(true);
	grid->enableY(true);
	grid->enableYMin(false);
	grid->attach(this);

	/*
	// SetCurrent
	m_setValueCurve = new QwtPlotCurve( "Set" );
	m_setValueCurve->setYAxis( currentAxisId );
	m_setValueCurve->setXAxis( QwtAxisId( QwtAxis::xBottom, 0 ) );
	m_setValueCurve->setStyle( QwtPlotCurve::Lines );
	m_setValueCurve->setPen( Qt::green, 2.0, Qt::SolidLine );
	m_setValueCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
	m_setValueCurve->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );
	//d_setCurrentCurve->setPaintAttribute( QwtPlotCurve::FilterPoints, true );
	m_setValueCurve->setData( new SetValueCurveData() );
	m_setValueCurve->attach( this );
	*/

	// Voltage
	/*
	m_currentCurve = new QwtPlotCurve( "Voltage" );
	m_currentCurve->setYAxis( voltageAxisId );
	//m_currentCurve->setXAxis( QwtAxisId( QwtAxis::xBottom, 0 ) ); // TODO: Multiaxis
	m_currentCurve->setXAxis( 0 );
	m_currentCurve->setStyle( QwtPlotCurve::Lines );
	m_currentCurve->setPen( Qt::green, 2.0, Qt::SolidLine );
	m_currentCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
	m_currentCurve->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );
	m_currentCurve->setData( new VoltageCurveData() );
	m_currentCurve->attach( this );
	*/

	// Value curve
	value_curve_ = new QwtPlotCurve("Current");
	value_curve_->setYAxis(currentAxisId);
	//value_curve_->setXAxis(QwtAxisId(QwtAxis::xBottom, 0)); // TODO: Multiaxis
	value_curve_->setXAxis(0);
	value_curve_->setStyle(QwtPlotCurve::Lines);
	value_curve_->setPen(Qt::red, 2.0, Qt::SolidLine);
	value_curve_->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	value_curve_->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
	value_curve_->setData(curve_data_);
	//value_curve_->setRawSamples();
	value_curve_->attach(this);
}

Plot::~Plot()
{
	delete valueDirectPainter_;
}

void Plot::start()
{
	timer_id_ = startTimer(plot_interval_);
}

void Plot::stop()
{
	killTimer(timer_id_);
}

void Plot::replot()
{
	//ReLoadProData::instance().lock();

	QwtPlot::replot();
	painted_points_ = curve_data_->size();

	//ReLoadProData::instance().unlock();
}

void Plot::set_x_interval(double x_start, double x_end)
{
	if (x_start != x_interval_.minValue() && x_end != x_interval_.maxValue()) {
		x_interval_.setInterval(x_start, x_end);
		setAxisScale(QwtPlot::xBottom,
			x_interval_.minValue(), x_interval_.maxValue());

		replot();
	}
}

void Plot::set_y_interval(double y_start, double y_end)
{
	if (y_start != y_interval_.minValue() && y_end != y_interval_.maxValue()) {
		y_interval_.setInterval(y_start, y_end);
		setAxisScale(QwtPlot::yLeft,
			y_interval_.minValue(), y_interval_.maxValue());

		replot();
	}
}

void Plot::update_curve()
{
	//ReLoadProData::instance().lock(); // TODO

	const int numPoints = curve_data_->size();
	if (numPoints > painted_points_) {
		qWarning() << QString("Plot::updateCurve(): numPoints = %1, painted_points_ = %2").arg(numPoints).arg(painted_points_);
		const bool clip = !canvas()->testAttribute(Qt::WA_PaintOnScreen);
		if (clip) {
			/*
				Depending on the platform setting a clip might be an important
				performance issue. F.e. for Qt Embedded this reduces the
				part of the backing store that has to be copied out - maybe
				to an unaccelerated frame buffer device.
			*/

			const QwtScaleMap xMap = canvasMap(value_curve_->xAxis());
			const QwtScaleMap yMap = canvasMap(value_curve_->yAxis());

			QRectF br = qwtBoundingRect(*value_curve_->data(),
				painted_points_ - 1, numPoints - 1);

			const QRect clipRect = QwtScaleMap::transform(xMap, yMap, br).toRect();
			valueDirectPainter_->setClipRegion(clipRect);
		}

		valueDirectPainter_->drawSeries(value_curve_,
			painted_points_ - 1, numPoints - 1);

		painted_points_ = numPoints;
	}

	//ReLoadProData::instance().unlock(); // TODO

	//replot();
}

void Plot::increment_x_interval()
{
	qWarning() << QString("Plot::increment_x_interval(): old min = %1, old max = %2").arg(x_interval_.minValue()).arg(x_interval_.maxValue());

	if (plot_mode_ == Plot::Additive) {
		// TODO: Calculate proper interval_length
		int interval_length = 30;
		x_interval_ = QwtInterval(x_interval_.minValue(),
			x_interval_.maxValue() + interval_length);

		qWarning() << QString("Plot::increment_x_interval(): new min = %1, new max = %2").arg(x_interval_.minValue()).arg(x_interval_.maxValue());
	}
	else if (plot_mode_ == Plot::Oscilloscope) {
		x_interval_ = QwtInterval(x_interval_.maxValue(),
			x_interval_.maxValue() + x_interval_.width());
	}

	//curve_data_->clearStaleValues(interval_.minValue()); // TODO

	if (plot_mode_ == Plot::Additive) {
		setAxisScale(QwtPlot::xBottom,
			x_interval_.minValue(), x_interval_.maxValue());
	}
	else if (plot_mode_ == Plot::Oscilloscope) {
		// To avoid, that the grid is jumping, we disable
		// the autocalculation of the ticks and shift them
		// manually instead.

		QwtScaleDiv scaleDiv = axisScaleDiv(QwtPlot::xBottom);
		scaleDiv.setInterval(x_interval_);

		for (int i = 0; i < QwtScaleDiv::NTickTypes; i++) {
			QList<double> ticks = scaleDiv.ticks(i);
			for (int j = 0; j < ticks.size(); j++) {
				ticks[j] += x_interval_.width();
			}
			scaleDiv.setTicks(i, ticks);
		}
		setAxisScaleDiv(QwtPlot::xBottom, scaleDiv);

		painted_points_ = 0;
	}
	else if (plot_mode_ == Plot::Rolling) {
	}
}

void Plot::increment_y_interval(QRectF boundaries)
{
	// TODO: Calculate proper interval_length
	double interval_length = 0.5;

	if (boundaries.bottom() < y_interval_.minValue())
		y_interval_.setMinValue(y_interval_.minValue() - interval_length);

	if (boundaries.top() > y_interval_.maxValue())
		y_interval_.setMaxValue(y_interval_.maxValue() + interval_length);

	setAxisScale(QwtPlot::yLeft,
		y_interval_.minValue(), y_interval_.maxValue());
}

void Plot::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == timer_id_) {

		bool intervals_changed = false;
		QRectF boundaries = curve_data_->boundingRect();

		// Check for x axis resize
		// TODO: this is for time only! Implement like y axis!
		if (boundaries.right() > x_interval_.maxValue()) {
			increment_x_interval();
			intervals_changed = true;
		}

		// Check for y axis resize
		if (boundaries.bottom() > y_interval_.minValue() ||
			boundaries.top() > y_interval_.maxValue()) {
			increment_y_interval(boundaries);
			intervals_changed = true;
		}

		if (intervals_changed)
			replot();

		update_curve();

		return;
	}

	QwtPlot::timerEvent(event);
}

void Plot::resizeEvent(QResizeEvent *event)
{
	valueDirectPainter_->reset();
	QwtPlot::resizeEvent(event);
}

void Plot::showEvent(QShowEvent *)
{
	replot();
}

bool Plot::eventFilter(QObject *object, QEvent *event)
{
	if (object == canvas() && event->type() == QEvent::PaletteChange) {
		//m_setValueCurve->setPen( Qt::green, 2.0, Qt::SolidLine );
		//m_currentCurve->setPen( Qt::red, 2.0, Qt::SolidLine );
		value_curve_->setPen(Qt::green, 2.0, Qt::SolidLine);
	}

	return QwtPlot::eventFilter(object, event);
}

} // namespace widgets
} // namespace sv
