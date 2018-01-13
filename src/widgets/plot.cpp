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
#include <qwt_curve_fitter.h>
#include <qwt_legend.h>
#include <qwt_painter.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_draw.h>
#include <qwt_symbol.h>

#include "plot.hpp"
#include "src/data/basecurve.hpp"

namespace sv {
namespace widgets {

class Canvas: public QwtPlotCanvas
{
public:
	Canvas(QwtPlot *plot = NULL) : QwtPlotCanvas(plot)
	{
		// The backing store is important, when working with widget
		// overlays ( f.e rubberbands for zooming ).
		// Here we don't have them and the internal
		// backing store of QWidget is good enough.

		setPaintAttribute(QwtPlotCanvas::BackingStore, false);
		setBorderRadius(10);

		if (QwtPainter::isX11GraphicsSystem()) {
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

		QLinearGradient gradient;
		gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
		gradient.setColorAt(0.0, QColor(0, 49, 110));
		gradient.setColorAt(1.0, QColor(0, 87, 174));

		pal.setBrush(QPalette::Window, QBrush(gradient));

		// QPalette::WindowText is used for the curve color
		pal.setColor(QPalette::WindowText, Qt::green);

		setPalette(pal);
	}
};

Plot::Plot(data::BaseCurve *curve_data, QWidget *parent) :
		QwtPlot(parent),
	curve_data_(curve_data),
	painted_points_(0),
	plot_interval_(200),
	timer_id_(-1)
{
	valueDirectPainter_ = new QwtPlotDirectPainter();

	this->setAutoReplot(false);
	this->setCanvas(new Canvas());

	// This must be done, because when the QwtPlot widget is directly or
	// indirectly in a (Main)Window, the minimum size is way to big
	this->setMinimumSize(250, 250);

	this->plotLayout()->setAlignCanvasToScales(true);

	QwtLegend *legend = new QwtLegend;
	legend->setDefaultItemMode(QwtLegendData::Checkable);
	this->insertLegend(legend, QwtPlot::BottomLegend);

	init_x_axis();
	init_y_axis();

	QwtPlotGrid *grid = new QwtPlotGrid();
	grid->setPen(Qt::gray, 0.0, Qt::DotLine);
	grid->enableX(true);
	grid->enableXMin(true);
	grid->enableY(true);
	grid->enableYMin(false);
	grid->attach(this);

	init_curve();
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

	if (curve_data_ != nullptr)
		painted_points_ = curve_data_->size();

	//ReLoadProData::instance().unlock();
}

void Plot::set_curve_data(data::BaseCurve *curve_data)
{
	curve_data_ = curve_data;
	this->init_x_axis();
	this->init_y_axis();
	this->init_curve();
}

int Plot::init_x_axis()
{
	x_axis_id_ = QwtPlot::xBottom;
	double min = 0.;
	double max = 0.;
	QString title("");
	if (curve_data_ != nullptr) {
		min = curve_data_->boundingRect().left();
		max = curve_data_->boundingRect().right();
		title = curve_data_->x_data_title();
	}

	x_interval_ = QwtInterval(min, max);

	this->setAxisTitle(x_axis_id_, title);
	this->setAxisScale(x_axis_id_, x_interval_.minValue(), x_interval_.maxValue());
	//setAxisAutoScale(x_axis_id_, true); // TODO: Not working!?

	return x_axis_id_;
}

int Plot::init_y_axis()
{
	y_axis_id_ = QwtPlot::yLeft;
	double min = 0.;
	double max = 0.;
	QString title("");
	if (curve_data_ != nullptr) {
		max = curve_data_->boundingRect().top();
		title = curve_data_->y_data_title();
	}

	y_interval_ = QwtInterval(min, max);
	//setAxesCount(QwtPlot::yLeft, 2); // TODO: Multiaxis
	//QwtAxisId y_axis_id = QwtAxisId(QwtAxis::yLeft, 0); // TODO: Multiaxis

	//setAxisVisible(y_axis_id_, true); // TODO: Multiaxis
	this->setAxisTitle(y_axis_id_, title);
	this->setAxisScale(y_axis_id_, y_interval_.minValue(), y_interval_.maxValue());
	this->setAxisAutoScale(y_axis_id_, false); // TODO: Not working!?

	return y_axis_id_;
}

void Plot::init_curve()
{
	if (curve_data_ == nullptr)
		return;

	value_curve_ = new QwtPlotCurve(curve_data_->y_data_quantity());
	//value_curve_->setXAxis(QwtAxisId(QwtAxis::yLeft, 0)); // TODO: Multiaxis
	value_curve_->setYAxis(y_axis_id_);
	value_curve_->setXAxis(x_axis_id_);
	value_curve_->setStyle(QwtPlotCurve::Lines);
	value_curve_->setPen(Qt::red, 2.0, Qt::SolidLine);
	value_curve_->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	value_curve_->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
	value_curve_->setData(curve_data_);
	//value_curve_->setRawSamples(); // TODO: is this an option?

	value_curve_->attach(this);
}

void Plot::set_x_interval(double x_start, double x_end)
{
	if (x_start != x_interval_.minValue() && x_end != x_interval_.maxValue()) {
		x_interval_.setInterval(x_start, x_end);
		setAxisScale(QwtPlot::xBottom,
			x_interval_.minValue(), x_interval_.maxValue());

		//replot();
	}
}

void Plot::set_y_interval(double y_start, double y_end)
{
	if (y_start != y_interval_.minValue() && y_end != y_interval_.maxValue()) {
		y_interval_.setInterval(y_start, y_end);
		setAxisScale(QwtPlot::yLeft,
			y_interval_.minValue(), y_interval_.maxValue());

		//replot();
	}
}

void Plot::add_marker()
{
	QwtSymbol *sym = new QwtSymbol(
		QwtSymbol::Diamond, QBrush(Qt::red), QPen(Qt::red), QSize(5,5));

	marker_ = new QwtPlotMarker(QString("Marker1"));
	marker_->setLabel(QwtText(QString("Marker1")));
	marker_->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
	marker_->setSymbol(sym);
	marker_->setLineStyle(QwtPlotMarker::Cross);
	marker_->setLinePen(Qt::green, 1, Qt::DotLine);
	marker_->setValue(2, 2);
	marker_->attach(this);

	replot();

	QwtPlotPicker *picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
		QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, canvas());
	picker->setStateMachine(new QwtPickerPolygonMachine());
	picker->setRubberBandPen(QColor(Qt::yellow));
	picker->setRubberBand(QwtPicker::CrossRubberBand);
	picker->setTrackerPen(QColor(Qt::white));
	connect(picker, SIGNAL(moved(QPoint)), this, SLOT(moved(QPoint)));
}

void Plot::on_marker_moved(QPoint p)
{
    double x_top = invTransform(QwtPlot::xTop, p.x());
    double y_left = invTransform(QwtPlot::yLeft, p.y());
    QwtText label = axisScaleDraw(QwtPlot::xBottom)->label(
		invTransform(QwtPlot::xBottom, p.x()));

    marker_->setValue(x_top, y_left);
    marker_->setLabel(label);

    replot();
}

void Plot::update_curve()
{
	//ReLoadProData::instance().lock(); // TODO

	if (!curve_data_)
		return;

	const int numPoints = curve_data_->size();
	if (numPoints > painted_points_) {
		//qWarning() << QString("Plot::updateCurve(): numPoints = %1, painted_points_ = %2").arg(numPoints).arg(painted_points_);
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
	/*
	qWarning() << QString("Plot::increment_x_interval(): old min = %1, old max = %2").
		arg(x_interval_.minValue()).arg(x_interval_.maxValue());
	*/

	if (plot_mode_ == Plot::Additive) {
		// TODO: Calculate proper interval_length
		int interval_length = 30;
		x_interval_ = QwtInterval(x_interval_.minValue(),
			x_interval_.maxValue() + interval_length);

		/*
		qWarning() << QString("Plot::increment_x_interval(): new min = %1, new max = %2").
			arg(x_interval_.minValue()).arg(x_interval_.maxValue());
		*/
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
	// TODO: Add some percent and round to full tick
	if (boundaries.bottom() < y_interval_.minValue())
		y_interval_.setMinValue(boundaries.bottom() - 0.5);
	else if (boundaries.top() > y_interval_.maxValue())
		y_interval_.setMaxValue(boundaries.top() + 0.5);

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
		if (boundaries.bottom() < y_interval_.minValue() ||
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
		value_curve_->setPen(Qt::green, 2.0, Qt::SolidLine);
	}

	return QwtPlot::eventFilter(object, event);
}

} // namespace widgets
} // namespace sv
