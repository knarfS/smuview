/*
 * This file is part of the SmuView project.
 * This file is based on the QWT Oscilloscope Example.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <QBoxLayout>
#include <QColor>
#include <QDebug>
#include <QEvent>
#include <QBoxLayout>
#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QPushButton>
#include <QRectF>
#include <QSize>
#include <QVBoxLayout>
#include <qwt_scale_widget.h>
#include <qwt_curve_fitter.h>
#include <qwt_date_scale_engine.h>
#include <qwt_legend.h>
#include <qwt_painter.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>

#include "scopeplot.hpp"
#include "src/data/datautil.hpp"
#include "src/ui/dialogs/plotcurveconfigdialog.hpp"
#include "src/ui/widgets/plot/axislocklabel.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"
#include "src/ui/widgets/plot/plotmagnifier.hpp"
#include "src/ui/widgets/plot/plotscalepicker.hpp"

using std::make_pair;

namespace sv {
namespace ui {
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

		setup_palette();
	}

private:
	void setup_palette()
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

ScopePlot::ScopePlot(uint64_t samplerate, int num_hdiv,
		sv::data::rational_t timebase, QWidget *parent) :
	QwtPlot(parent),
	samplerate_(samplerate),
	num_hdiv_(num_hdiv),
	timebase_(timebase),
	plot_interval_(200), // TODO
	timer_id_(-1),
	trigger_marker_(nullptr)
	/*,
	active_marker_(nullptr),
	markers_label_(nullptr),
	marker_select_picker_(nullptr),
	marker_move_picker_(nullptr)
	*/
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

	// Disable all y axis to have a known state for init_y_axis()
	this->enableAxis(QwtPlot::yLeft, false);
	this->enableAxis(QwtPlot::yRight, false);
}

ScopePlot::~ScopePlot()
{
	//qWarning() << "ScopePlot::~ScopePlot() for " << curve_data_->name();
	this->stop();
	for (auto &direct_painter_pair : plot_direct_painter_map_) {
		delete direct_painter_pair.second;
	}
}

void ScopePlot::start()
{
	timer_id_ = startTimer(plot_interval_);
}

void ScopePlot::stop()
{
	//qWarning() << "ScopePlot::stop() for " << curve_data_->name();
	killTimer(timer_id_);
}

void ScopePlot::replot()
{
	//qWarning() << "ScopePlot::replot()";

	for (const auto &curve_data_pair : curve_data_map_) {
		painted_points_map_[curve_data_pair.second] = 0;
	}

	QwtPlot::replot();
}

bool ScopePlot::add_channel(QString name)
{
	// Check y axis
	int y_axis_id = this->init_y_axis(name);
	if (y_axis_id < 0)
		return false;

	// Check x axis
	int x_axis_id = this->init_x_axis();
	if (x_axis_id < 0)
		return false;

	QPen pen;
	pen.setColor(Qt::yellow);
	pen.setWidthF(2.0);
	pen.setStyle(Qt::SolidLine);
	pen.setCosmetic(false);

	QwtPlotCurve *plot_curve = new QwtPlotCurve(name);
	plot_curve->setYAxis(y_axis_id);
	plot_curve->setXAxis(x_axis_id);
	plot_curve->setStyle(QwtPlotCurve::Lines);
	plot_curve->setPen(pen);
	plot_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	plot_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
	//plot_curve->setRawSamples(); // TODO: is this an option?
	plot_curve->attach(this);
	plot_curve_map_.insert(make_pair(y_axis_id, plot_curve));

	QwtPlotDirectPainter *direct_painter = new QwtPlotDirectPainter();
	plot_direct_painter_map_.insert(make_pair(y_axis_id, direct_painter));

	return true;
}

bool ScopePlot::show_curve(QString ch_name,
	widgets::plot::BaseCurveData *curve_data)
{
	assert(curve_data);

	int y_axis_id = y_axis_id_map_[ch_name];
	plot_curve_map_[y_axis_id]->setData(curve_data);
	curve_data_map_[y_axis_id] = curve_data;
	if (painted_points_map_.count(curve_data) == 0)
		painted_points_map_.insert(make_pair(curve_data, 0));
	else
		painted_points_map_[curve_data] = 0;

	QwtPlot::replot();

	return true;
}

void ScopePlot::update_samplerate(const QVariant samplerate)
{
	samplerate_ = samplerate.toULongLong();
	update_x_interval();
}

void ScopePlot::update_num_hdiv(const QVariant num_hdiv)
{
	num_hdiv_ = num_hdiv.toInt();
	update_x_interval();
}

void ScopePlot::update_timebase(const QVariant timebase)
{
	timebase_ = timebase.value<data::rational_t>();
	update_x_interval();
}

void ScopePlot::update_trigger_source(const QVariant trigger_source)
{
	trigger_source_ = trigger_source.toString();
	update_trigger_marker();
}

void ScopePlot::update_trigger_level(const QVariant trigger_level)
{
	trigger_level_ = trigger_level.toDouble();
	update_trigger_marker();
}

/*
void ScopePlot::add_marker(plot::BaseCurveData *curve_data)
{
	assert(curve_data);

	QwtPlotCurve *plot_curve = plot_curve_map_[curve_data];

	QString marker_name = QString("M%1").arg(markers_.size()+1);
	QwtSymbol *marker_sym = new QwtSymbol(
		QwtSymbol::Diamond, QBrush(Qt::red), QPen(Qt::red), QSize(9, 9));

	QwtPlotMarker *marker = new QwtPlotMarker(marker_name);
	marker->setSymbol(marker_sym);
	marker->setLineStyle(QwtPlotMarker::Cross);
	marker->setLinePen(Qt::black, 1, Qt::DotLine);
	// Initial marker position is at the end of the curve
	marker->setValue(curve_data->sample(curve_data->size()-1));
	marker->setLabel(QwtText(marker_name));
	marker->setLabelAlignment(Qt::AlignTop | Qt::AlignRight);
	marker->setXAxis(plot_curve->xAxis());
	marker->setYAxis(plot_curve->yAxis());
	marker->attach(this);

	markers_.push_back(marker);
	marker_map_.insert(make_pair(marker, curve_data));
	active_marker_ = marker;

	// Add pickers for _all_ markers, no matter of the axis
	if (!marker_select_picker_) {
		// Use QwtPlot::xBottom and QwtPlot::yLeft as axis. We calculate the
		// canvas positions for the markers in on_marker_selected()
		marker_select_picker_ = new QwtPlotPicker(
			QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::NoRubberBand,
			QwtPicker::AlwaysOff, this->canvas());
		marker_select_picker_->setStateMachine(new QwtPickerClickPointMachine());
		connect(marker_select_picker_, SIGNAL(selected(const QPointF &)),
			this, SLOT(on_marker_selected(const QPointF)));
	}
	if (!marker_move_picker_) {
		// Use QwtPlot::xBottom and QwtPlot::yLeft as axis. We calculate the
		// canvas positions for the markers in on_marker_moved()
		marker_move_picker_ = new QwtPlotPicker(
			QwtPlot::xBottom, QwtPlot::yLeft,
			QwtPlotPicker::NoRubberBand, QwtPicker::AlwaysOff, this->canvas());
		marker_move_picker_->setStateMachine(new QwtPickerDragPointMachine());
		connect(marker_move_picker_, SIGNAL(moved(QPointF)),
			this, SLOT(on_marker_moved(QPointF)));
	}
	/ *
	 * TODO: Maybe we could use a QwtPickerTrackerMachine for mouse movement.
	 * This way we can avoid the mouse click event (problems with QwtPlotPanner)
	 * and also highlight the marker that in the "selectable range" of the
	 * mouse pointer. Maybe this would be a performance issue?
	 * /

	update_markers_label();
	replot();
}
*/

/*
void ScopePlot::add_diff_marker(QwtPlotMarker *marker1, QwtPlotMarker *marker2)
{
	if (!marker1 || !marker2)
		return;

	diff_markers_.push_back(make_pair(marker1, marker2));

	update_markers_label();
	replot();
}
*/

/*
// TODO: implement remove marker call
void ScopePlot::remove_marker()
{
	// If last marker of this axis.
	disconnect(marker_select_picker_, SIGNAL(selected(const QPointF &)),
		this, SLOT(on_marker_selected(const QPointF)));
	delete marker_select_picker_;
	marker_select_picker_ = nullptr;
	disconnect(marker_move_picker_, SIGNAL(moved(QPointF)),
		this, SLOT(on_marker_moved(QPointF)));
	delete marker_move_picker_;
	marker_move_picker_ = nullptr;
}
*/

/*
void ScopePlot::on_marker_selected(const QPointF mouse_pos)
{
	if (markers_.empty())
		return;

	// Mouse canvas coordinates. Use QwtPlot::xBottom and QwtPlot::yLeft
	// as axis for marker_select_picker_.
	const double mouse_canvas_x = transform(QwtPlot::xBottom, mouse_pos.x());
	const double mouse_canvas_y = transform(QwtPlot::yLeft, mouse_pos.y());

	double d_min = 15.; // Minimum distance to marker for selecting
	double d_lowest = 1.0e10;
	QwtPlotMarker *selected_marker = nullptr;
	for (const auto &marker : markers_) {
		// Marker canvas coordinates. Use axis ids form plot.
		QwtPlotCurve *plot_curve = plot_curve_map_[marker_map_[marker]];
		const double marker_canvas_x =
			transform(plot_curve->xAxis(), marker->xValue());
		const double marker_canvas_y =
			transform(plot_curve->yAxis(), marker->yValue());

		const double d_x = marker_canvas_x - mouse_canvas_x;
		const double d_y = marker_canvas_y - mouse_canvas_y;
		const double d = qSqrt(qwtSqr(d_x) + qwtSqr(d_y));
		if (d <= d_min && d < d_lowest) {
			d_lowest = d;
			selected_marker = marker;
		}
	}

	if (selected_marker) {
		active_marker_ = selected_marker;
	}
	else {
		active_marker_ = nullptr;
	}

	// TODO: connect/disconnect marker_move_picker_
}
*/

/*
void ScopePlot::on_marker_moved(const QPointF mouse_pos)
{
	if (!active_marker_)
		return;

	plot::BaseCurveData *curve_data = marker_map_[active_marker_];
	QPointF marker_pos = curve_data->closest_point(mouse_pos, NULL);
	active_marker_->setValue(marker_pos);

	update_markers_label();
	replot();
}
*/

void ScopePlot::on_legend_clicked(const QVariant &item_info, int index)
{
	(void)index;

	QwtPlotItem *plot_item = infoToItem(item_info);
	if (plot_item) {
		QwtPlotCurve *plot_curve = (QwtPlotCurve *)plot_item;
		ui::dialogs::PlotCurveConfigDialog dlg(plot_curve);
		dlg.exec();
	}
}

int ScopePlot::init_x_axis()
{
	int x_axis_id = QwtPlot::xBottom;

	//QString title = curve_data->x_data_title();
	//this->setAxisTitle(x_axis_id, title);

	//this->setAxisScale(x_axis_id, 0, max); // TODO: init?
	//this->setAxisAutoScale(x_axis_id, true); // TODO: Not working!?
	if (!this->axisEnabled(x_axis_id))
		this->enableAxis(x_axis_id);

	return x_axis_id;
}

int ScopePlot::init_y_axis(QString ch_name)
{
	if (y_axis_id_map_.count(ch_name) > 0)
		return -1;

	int y_axis_id = -1;
	if (!this->axisEnabled(QwtPlot::yLeft))
		y_axis_id = QwtPlot::yLeft;
	else if (!this->axisEnabled(QwtPlot::yLeft))
		y_axis_id = QwtPlot::yRight;
	else
		return -1;

	y_axis_id_map_.insert(make_pair(ch_name, y_axis_id));
	this->setAxisTitle(y_axis_id, ch_name);
	this->setAxisScale(y_axis_id, 0., 0.);
	this->setAxisAutoScale(y_axis_id, false); // TODO: Not working!?
	this->enableAxis(y_axis_id);

	return y_axis_id;
}

void ScopePlot::update_curves()
{
	// TODO: mutex for curve_data_map_
	for (const auto &curve_data_pairs : curve_data_map_) {
		const size_t painted_points = painted_points_map_[curve_data_pairs.second];
		const size_t num_points = curve_data_pairs.second->size();
		if (num_points > painted_points) {
			QwtPlotCurve *plot_curve = plot_curve_map_[curve_data_pairs.first];
			QwtPlotDirectPainter *direct_painter =
				plot_direct_painter_map_[curve_data_pairs.first];

			//qWarning() << QString("ScopePlot::updateCurve(): num_points = %1, painted_points = %2").arg(num_points).arg(painted_points);
			const bool clip = !canvas()->testAttribute(Qt::WA_PaintOnScreen);
			if (clip) {
				/*
				 * NOTE:
				 * Depending on the platform setting a clip might be an
				 * important performance issue. F.e. for Qt Embedded this
				 * reduces the part of the backing store that has to be copied
				 * out - maybe to an unaccelerated frame buffer device.
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

			painted_points_map_[curve_data_pairs.second] = num_points;
		}

		//replot();
	}
}

void ScopePlot::update_intervals()
{
	bool intervals_changed = false;

	if (update_x_interval())
		intervals_changed = true;
	for (const auto &curve_data_pair : curve_data_map_) {
		if (update_y_interval(curve_data_pair.first))
			intervals_changed = true;
	}

	if (intervals_changed)
		replot();
}

bool ScopePlot::update_x_interval()
{
	if (num_hdiv_ > 0 && timebase_.second > 0) {
		double timespan = num_hdiv_ * (timebase_.first / (double)timebase_.second);
		qWarning() << "ScopePlot::update_x_interval(): timespan = " << timespan;
		this->setAxisScale(QwtPlot::xBottom, 0, timespan); // TODO: axis ID
	}
	else {
		// Find the x max value for the curves
		// TODO: old max?
		double min = 0.;
		double max = 0.;
		for (const auto &curve_data_pair : curve_data_map_) {
			QRectF boundaries = curve_data_pair.second->boundingRect();
			//if (boundaries.left() < min)
			//	min = boundaries.left();
			if (boundaries.right() > max)
				max = boundaries.right();
		}
		this->setAxisScale(QwtPlot::xBottom, min, max); // TODO: axis ID
		qWarning() << "ScopePlot::update_x_interval(): min = " << min;
		qWarning() << "ScopePlot::update_x_interval(): max = " << max;
	}

	/*
	QwtInterval x_interval = this->axisInterval(QwtPlot::xBottom);
	if (x_interval.maxValue() > timespan)
		return false;
	*/

	/*
	 * NOTE:
	 * To avoid, that the grid is jumping, we disable the autocalculation
	 * of the ticks and shift them manually instead.
	QwtScaleDiv scaleDiv = this->axisScaleDiv(QwtPlot::xBottom); // TODO: axis ID
	scaleDiv.setInterval(0, timespan);
	for (int i = 0; i < QwtScaleDiv::NTickTypes; i++) {
		QList<double> ticks = scaleDiv.ticks(i);
		for (int j = 0; j < ticks.size(); j++) {
			ticks[j] += x_interval.width();
		}
		scaleDiv.setTicks(i, ticks);
	}
	this->setAxisScaleDiv(QwtPlot::xBottom, scaleDiv); // TODO: axis ID
	*/

	for (const auto &curve_data_pair : curve_data_map_) {
		painted_points_map_[curve_data_pair.second] = 0; // TODO?
	}

	return true;
}

bool ScopePlot::update_y_interval(int y_axis_id)
{
	QRectF boundaries = curve_data_map_[y_axis_id]->boundingRect();
	QwtInterval y_interval = this->axisInterval(y_axis_id);
	double min = y_interval.minValue();
	double max = y_interval.maxValue();
	bool interval_changed = false;

	qWarning() << "ScopePlot::update_y_interval() for b.bottom < y_int.min = " <<
		boundaries.bottom() << " < " << min;
	qWarning() << "ScopePlot::update_y_interval() for b.top > y_int.max = " <<
		boundaries.top() << " > " << max;

	if (boundaries.bottom() < min) {
		// new value + 10%
		min = boundaries.bottom() + (boundaries.bottom() * 0.1);
		interval_changed = true;
	}
	if (boundaries.top() > max) {
		// new value + 10%
		max = boundaries.top() + (boundaries.top() * 0.1);
		interval_changed = true;
	}

	if (interval_changed ) {
		/*
		qWarning() <<
			QString("ScopePlot::update_y_interval(): new min = %1, new max = %2").
			arg(min).arg(max);
		*/
		setAxisScale(y_axis_id, min, max);
	}

	return interval_changed;
}

bool ScopePlot::update_trigger_marker()
{
	// TODO: trigger source

	if (!trigger_marker_) {
		QwtSymbol *marker_sym = new QwtSymbol(QwtSymbol::RTriangle,
			QBrush(Qt::yellow), QPen(Qt::yellow), QSize(20, 20));

		trigger_marker_ = new QwtPlotMarker();
		trigger_marker_->setSymbol(marker_sym);
		trigger_marker_->setLineStyle(QwtPlotMarker::HLine);
		trigger_marker_->setLinePen(Qt::yellow, 1, Qt::DashDotLine);
		trigger_marker_->setLabelOrientation(Qt::Horizontal);
		trigger_marker_->setLabelAlignment(Qt::AlignTop | Qt::AlignRight);
		trigger_marker_->setXAxis(QwtPlot::xBottom);
		trigger_marker_->setYAxis(QwtPlot::yLeft);
		trigger_marker_->attach(this);
	}

	trigger_marker_->setLabel(QwtText(trigger_source_));
	trigger_marker_->setValue(0., trigger_level_);

	replot();

	return true;
}

/*
void ScopePlot::update_markers_label()
{
	if (!markers_label_) {
		markers_label_ = new QwtPlotTextLabel();
		markers_label_->setMargin(5);
		markers_label_->attach(this);
	}

	QString table("<table>");

	for (const auto &marker : markers_) {
		table.append("<tr>");
		table.append(QString("<td width=\"50\" align=\"left\">%1:</td>").
			arg(marker->title().text()));
		table.append(QString("<td width=\"70\" align=\"right\">%2 %3</td>").
			arg(marker->yValue()).
			arg(marker_map_[marker]->y_data_unit()));
		table.append(QString("<td width=\"70\" align=\"right\">%4 %5</td>").
			arg(marker->xValue()).
			arg(marker_map_[marker]->x_data_unit()));
		table.append("</tr>");
	}

	for (const auto &marker_pair : diff_markers_) {
		double d_x = marker_pair.first->xValue() - marker_pair.second->xValue();
		double d_y = marker_pair.first->yValue() - marker_pair.second->yValue();

		QString x_unit("");
		QString m1_x_unit = marker_map_[marker_pair.first]->x_data_unit();
		QString m2_x_unit = marker_map_[marker_pair.second]->x_data_unit();
		if (m1_x_unit == m1_x_unit)
			x_unit = m1_x_unit;

		QString y_unit("");
		QString m1_y_unit = marker_map_[marker_pair.first]->y_data_unit();
		QString m2_y_unit = marker_map_[marker_pair.second]->y_data_unit();
		if (m1_y_unit == m2_y_unit)
			y_unit = m1_y_unit;

		table.append("<tr>");
		table.append(QString("<td width=\"50\" align=\"left\">%1 - %2:</td>").
			arg(marker_pair.first->title().text()).
			arg(marker_pair.second->title().text()));
		table.append(QString("<td width=\"70\" align=\"right\">%2 %3</td>").
			arg(d_y).arg(y_unit));
		table.append(QString("<td width=\"70\" align=\"right\">%4 %5</td>").
			arg(d_x).arg(x_unit));
		table.append("</tr>");
	}

	table.append("</table>");

	QwtText text = QwtText(table);
	text.setPaintAttribute(QwtText::PaintBackground, true);
	QColor c(Qt::gray);
	c.setAlpha(200);
	text.setBackgroundBrush(c);
	text.setBorderRadius(3);
	QPen pen;
	pen.setColor(Qt::black);
	pen.setWidthF(1.0);
	pen.setStyle(Qt::SolidLine);
	text.setBorderPen(pen);
	text.setRenderFlags(Qt::AlignBottom | Qt::AlignHCenter);

	markers_label_->setText(text);
}
*/

void ScopePlot::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == timer_id_) {
		update_intervals();
		update_curves();
		return;
	}

	QwtPlot::timerEvent(event);
}

void ScopePlot::resizeEvent(QResizeEvent *event)
{
	for (auto &direct_painter_pair : plot_direct_painter_map_) {
		direct_painter_pair.second->reset();
	}

	QwtPlot::resizeEvent(event);
}

void ScopePlot::showEvent(QShowEvent *)
{
	replot();
}

bool ScopePlot::eventFilter(QObject *object, QEvent *event)
{
	return QwtPlot::eventFilter(object, event);
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
