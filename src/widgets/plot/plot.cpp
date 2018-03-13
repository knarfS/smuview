/*
 * This file is part of the SmuView project.
 * This file is based on the QWT Oscilloscope Example.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <utility>

#include <QDebug>
#include <QEvent>
#include <QPen>
#include <QRectF>
#include <qwt_curve_fitter.h>
#include <qwt_legend.h>
#include <qwt_painter.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_magnifier.h>
#include <qwt_scale_draw.h>
#include <qwt_symbol.h>

#include "plot.hpp"
#include "src/dialogs/plotcurveconfigdialog.hpp"
#include "src/widgets/plot/basecurve.hpp"
#include "src/widgets/plot/plotscalepicker.hpp"

using std::make_pair;

namespace sv {
namespace widgets {
namespace plot {

class Canvas : public QwtPlotCanvas
{
public:
	Canvas(QwtPlot *plot = NULL) : QwtPlotCanvas(plot)
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

Plot::Plot(QWidget *parent) : QwtPlot(parent),
	x_axis_fixed_(false),
	y_axis_fixed_(false),
	plot_interval_(200),
	timer_id_(-1)
{
	this->setAutoReplot(false);
	this->setCanvas(new Canvas());

	// This must be done, because when the QwtPlot widget is directly or
	// indirectly in a (Main)Window, the minimum size is way to big
	this->setMinimumSize(250, 250);

	this->plotLayout()->setAlignCanvasToScales(true);

	QwtLegend *legend = new QwtLegend;
	legend->setDefaultItemMode(QwtLegendData::Clickable);
	this->insertLegend(legend, QwtPlot::BottomLegend);
	connect(legend, SIGNAL(clicked(const QVariant &, int)),
		this, SLOT(on_legend_clicked(const QVariant &, int)));

	QwtPlotGrid *grid = new QwtPlotGrid();
	grid->setPen(Qt::gray, 0.0, Qt::DotLine);
	grid->enableX(true);
	grid->enableXMin(true);
	grid->enableY(true);
	grid->enableYMin(false);
	grid->attach(this);

    // Zooming and panning via the axes
    (void)new PlotScalePicker(this);
	// Panning via the canvas
	//(void)new QwtPlotPanner(this->canvas());
	// Zooming via the canvas
	//(void)new QwtPlotMagnifier(this->canvas());
}

Plot::~Plot()
{
	//qWarning() << "Plot::~Plot() for " << curve_data_->name();
	this->stop();
	for (auto direct_painter_pair : plot_direct_painter_map_) {
		delete direct_painter_pair.second;
	}
}

void Plot::start()
{
	timer_id_ = startTimer(plot_interval_);
}

void Plot::stop()
{
	//qWarning() << "Plot::stop() for " << curve_data_->name();
	killTimer(timer_id_);
}

void Plot::replot()
{
	//ReLoadProData::instance().lock();

	//qWarning() << "Plot::replot()";

	for (auto curve : curves_) {
		painted_points_map_[curve] = 0;
	}

	QwtPlot::replot();


	//ReLoadProData::instance().unlock();
}

void Plot::add_curve(widgets::plot::BaseCurve *curve)
{
	assert(curve);

	// Check y axis
	int y_axis_id = this->init_y_axis(curve);

	// Check x axis
	int x_axis_id = this->init_x_axis(curve);

	curves_.push_back(curve);

	QPen pen;
	pen.setColor(curve->color());
	pen.setWidthF(2.0);
	pen.setStyle(Qt::SolidLine);
	pen.setCosmetic(false);

	QwtPlotCurve *plot_curve = new QwtPlotCurve(curve->y_data_quantity());
	//plot_curve->setXAxis(QwtAxisId(QwtAxis::yLeft, 0)); // TODO: Multiaxis
	plot_curve->setYAxis(y_axis_id);
	plot_curve->setXAxis(x_axis_id);
	plot_curve->setStyle(QwtPlotCurve::Lines);
	plot_curve->setPen(pen);
	plot_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	plot_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
	plot_curve->setData(curve);
	//plot_curve->setRawSamples(); // TODO: is this an option?
	plot_curve->attach(this);
	plot_curve_map_.insert(make_pair(curve, plot_curve));

	QwtPlotDirectPainter *direct_painter = new QwtPlotDirectPainter();
	plot_direct_painter_map_.insert(make_pair(curve, direct_painter));

	painted_points_map_.insert(make_pair(curve, 0));

	QwtPlot::replot();
}

int Plot::init_x_axis(widgets::plot::BaseCurve *curve)
{
	assert(curve);

	int x_axis_id = QwtPlot::xBottom;
	double min = curve->boundingRect().left();
	double max = curve->boundingRect().right();
	QString title = curve->x_data_title();

	this->enableAxis(x_axis_id);
	this->setAxisTitle(x_axis_id, title);
	this->setAxisScale(x_axis_id, min, max);
	//setAxisAutoScale(x_axis_id, true); // TODO: Not working!?
	this->enableAxis(x_axis_id);

	return x_axis_id;
}

int Plot::init_y_axis(widgets::plot::BaseCurve *curve)
{
	assert(curve);

	int y_axis_id = -1;
	if (curves_.size() == 0)
		y_axis_id = QwtPlot::yLeft;
	else
		y_axis_id = QwtPlot::yRight;

	double min = 0.;
	double max = curve->boundingRect().top();
	QString title = curve->y_data_title();

	qWarning() << "Plot::init_y_axis(): title = " << title << ", y_axis_id = " << y_axis_id << ", min = " << min << ", max = " << max;

	//setAxesCount(QwtPlot::yLeft, 2); // TODO: Multiaxis
	//QwtAxisId y_axis_id = QwtAxisId(QwtAxis::yLeft, 0); // TODO: Multiaxis

	//this->setAxisVisible(y_axis_id, true); // TODO: Multiaxis
	this->setAxisTitle(y_axis_id, title);
	this->setAxisScale(y_axis_id, min, max);
	this->setAxisAutoScale(y_axis_id, false); // TODO: Not working!?
	this->enableAxis(y_axis_id);

	QwtInterval *y_interval = new QwtInterval(min, max);
	y_axis_interval_map_.insert(make_pair(y_axis_id, y_interval));
	curve_y_interval_map_.insert(make_pair(curve, y_interval));
	curve_y_axis_id_map_.insert(make_pair(curve, y_axis_id));

	return y_axis_id;
}

void Plot::set_x_interval(double x_start, double x_end)
{
	if (x_start != x_interval_.minValue() && x_end != x_interval_.maxValue()) {
		x_interval_.setInterval(x_start, x_end);
		x_axis_fixed_ = true;
		setAxisScale(QwtPlot::xBottom,
			x_interval_.minValue(), x_interval_.maxValue());

		replot();
	}
}

void Plot::set_y_interval(int y_axis_id, double y_start, double y_end)
{
	QwtInterval *y_interval = y_axis_interval_map_[y_axis_id];
	if (y_start != y_interval->minValue() && y_end != y_interval->maxValue()) {
		y_interval->setInterval(y_start, y_end);
		y_axis_fixed_ = true;
		setAxisScale(y_axis_id, y_interval->minValue(), y_interval->maxValue());

		replot();
	}
}

void Plot::set_x_axis_fixed(const bool fixed)
{
	x_axis_fixed_ = fixed;
}

void Plot::set_y_axis_fixed(const bool fixed)
{
	y_axis_fixed_ = fixed;
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

void Plot::on_legend_clicked(const QVariant &item_info, int index)
{
	(void)index;

	QwtPlotItem *plot_item = infoToItem(item_info);
    if (plot_item) {
		QwtPlotCurve *plot_curve = (QwtPlotCurve *)plot_item;
		dialogs::PlotCurveConfigDialog dlg(plot_curve);
		dlg.exec();
	}
}

void Plot::update_curves()
{
	for (plot::BaseCurve *curve : curves_) {
		//ReLoadProData::instance().lock(); // TODO

		const size_t painted_points = painted_points_map_[curve];
		const size_t num_points = curve->size();
		if (num_points > painted_points) {
			QwtPlotCurve *plot_curve = plot_curve_map_[curve];
			QwtPlotDirectPainter *direct_painter = plot_direct_painter_map_[curve];

			//qWarning() << QString("Plot::updateCurve(): num_points = %1, painted_points = %2").arg(num_points).arg(painted_points);
			const bool clip = !canvas()->testAttribute(Qt::WA_PaintOnScreen);
			if (clip) {
				/*
					Depending on the platform setting a clip might be an
					important performance issue. F.e. for Qt Embedded this
					reduces the part of the backing store that has to be copied
					out - maybe to an unaccelerated frame buffer device.
				*/

				const QwtScaleMap x_map = canvasMap(plot_curve->xAxis());
				const QwtScaleMap y_map = canvasMap(plot_curve->yAxis());
				QRectF br = qwtBoundingRect(*plot_curve->data(),
					painted_points - 1, num_points - 1);
				const QRect clip_rect =
					QwtScaleMap::transform(x_map, y_map, br).toRect();

				direct_painter->setClipRegion(clip_rect);
			}

			direct_painter->drawSeries(plot_curve,
				painted_points - 1, num_points - 1);

			painted_points_map_[curve] = num_points;
		}

		//ReLoadProData::instance().unlock(); // TODO

		//replot();
	}
}

void Plot::update_intervals()
{
	bool intervals_changed = false;

	for (plot::BaseCurve *curve : curves_) {
		//qWarning() << "Plot::timerEvent() for " << curve->name();
		QRectF boundaries = curve->boundingRect();

		// Check for x axis resize
		if (boundaries.left() < x_interval_.minValue() ||
				boundaries.right() > x_interval_.maxValue()) {
			qWarning() << "Plot::timerEvent(): increment_x_interval()";
			increment_x_interval(boundaries);
			intervals_changed = true;
		}

		// Check for y axis resize
		QwtInterval *y_interval = curve_y_interval_map_[curve];
		//qWarning() << "Plot::timerEvent() for " << curve->name() << " b.bottom < y_int.min = " << boundaries.bottom() << " < " << y_interval->minValue();
		//qWarning() << "Plot::timerEvent() for " << curve->name() << " b.top > y_int.max = " << boundaries.top() << " > " << y_interval->maxValue();
		if (boundaries.bottom() < y_interval->minValue() ||
				boundaries.top() > y_interval->maxValue()) {
			qWarning() << "Plot::timerEvent(): increment_y_interval()";
			increment_y_interval(curve, boundaries);
			intervals_changed = true;
		}
	}

	if (intervals_changed)
		replot();
}

void Plot::increment_x_interval(QRectF boundaries)
{
	if (x_axis_fixed_)
		return;

	//qWarning() << QString("Plot::increment_x_interval(): old min = %1, old max = %2").arg(x_interval_.minValue()).arg(x_interval_.maxValue());
	if (plot_mode_ == PlotModes::Additive) {
		//qWarning() << "Plot::timerEvent() for b.left < x_int.min = " << boundaries.left() << " < " << x_interval_.minValue();
		if (boundaries.left() < x_interval_.minValue()) {
			// TODO: Calculate proper interval_length
			x_interval_.setMinValue(x_interval_.minValue());
		}
		//qWarning() << "Plot::timerEvent() for b.right > x_int.max = " << boundaries.right() << " > " << x_interval_.maxValue();
		if (boundaries.right() > x_interval_.maxValue()) {
			// TODO: Calculate proper interval_length
			int interval_length = 30;
			x_interval_.setMaxValue(x_interval_.maxValue() + interval_length);
		}
		/*
		qWarning() <<
			QString("Plot::increment_x_interval(): new min = %1, new max = %2").
			arg(x_interval_.minValue()).arg(x_interval_.maxValue());
		*/
	}
	else if (plot_mode_ == PlotModes::Rolling) {
	}
	else if (plot_mode_ == PlotModes::Oscilloscope) {
		x_interval_ = QwtInterval(x_interval_.maxValue(),
			x_interval_.maxValue() + x_interval_.width());
	}

	//curve_data_->clearStaleValues(interval_.minValue()); // TODO

	if (plot_mode_ == PlotModes::Additive) {
		setAxisScale(QwtPlot::xBottom,
			x_interval_.minValue(), x_interval_.maxValue());
	}
	else if (plot_mode_ == PlotModes::Rolling) {
	}
	else if (plot_mode_ == PlotModes::Oscilloscope) {
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

		//painted_points_ = 0;
	}
}

void Plot::increment_y_interval(plot::BaseCurve *curve, QRectF boundaries)
{
	if (y_axis_fixed_)
		return;

	QwtInterval *y_interval = curve_y_interval_map_[curve];
	int y_axis_id = curve_y_axis_id_map_[curve];

	// TODO: Add some percent and round to full tick
	if (boundaries.bottom() < y_interval->minValue())
		y_interval->setMinValue(boundaries.bottom() - 0.5);
	else if (boundaries.top() > y_interval->maxValue())
		y_interval->setMaxValue(boundaries.top() + 0.5);

	setAxisScale(y_axis_id, y_interval->minValue(), y_interval->maxValue());
}

void Plot::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == timer_id_) {
		update_intervals();
		update_curves();
		return;
	}

	QwtPlot::timerEvent(event);
}

void Plot::resizeEvent(QResizeEvent *event)
{
	for (auto direct_painter_pair : plot_direct_painter_map_) {
		direct_painter_pair.second->reset();
	}

	QwtPlot::resizeEvent(event);
}

void Plot::showEvent(QShowEvent *)
{
	replot();
}

bool Plot::eventFilter(QObject *object, QEvent *event)
{
	return QwtPlot::eventFilter(object, event);
}

} // namespace plot
} // namespace widgets
} // namespace sv
