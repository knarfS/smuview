/*
 * This file is part of the SmuView project.
 * This file is based on the QWT Oscilloscope Example.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include <utility>

#include <QtMath>
#include <QBoxLayout>
#include <QDateTime>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QPushButton>
#include <QRectF>
#include <QSize>
#include <QUuid>
#include <QVariant>
#include <QVBoxLayout>
#include <qwt_scale_widget.h>
#include <qwt_curve_fitter.h>
#include <qwt_date_scale_engine.h>
#include <qwt_legend.h>
#include <qwt_math.h>
#include <qwt_painter.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_grid.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
// This header uses deprecated declarations, disable checks.
#include <qwt_plot_layout.h>
#pragma GCC diagnostic pop
#include <qwt_plot_textlabel.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_map.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>

#include "plot.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/dialogs/plotcurveconfigdialog.hpp"
#include "src/ui/widgets/plot/axislocklabel.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"
#include "src/ui/widgets/plot/curve.hpp"
#include "src/ui/widgets/plot/plotmagnifier.hpp"
#include "src/ui/widgets/plot/plotscalepicker.hpp"
#include "src/ui/widgets/plot/timecurvedata.hpp"
#include "src/ui/widgets/plot/xycurvedata.hpp"

using std::make_pair;
using std::shared_ptr;
using std::string;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

class Canvas : public QwtPlotCanvas
{
public:
	explicit Canvas(QwtPlot *plot = nullptr) : QwtPlotCanvas(plot)
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

Plot::Plot(Session &session, QWidget *parent) : QwtPlot(parent),
	session_(session),
	plot_interval_(200),
	timer_id_(-1),
	time_span_(120.),
	add_time_(30.),
	active_marker_(nullptr),
	markers_label_(nullptr),
	markers_label_alignment_(Qt::AlignBottom | Qt::AlignHCenter),
	marker_select_picker_(nullptr),
	marker_move_picker_(nullptr)
{
	this->setAutoReplot(false);
	this->setCanvas(new Canvas());

	// This must be done, because when the QwtPlot widget is directly or
	// indirectly in a (Main)Window, therefor the minimum size is way to big.
	this->setMinimumSize(250, 250);
	//qWarning() << "Plot::Plot(): sizeHint() = " << sizeHint() <<
	//	", minimumSizeHint() = " << minimumSizeHint();
	//qWarning() << "Plot::Plot(): size() = " << size() <<
	//	", minimumSize() = " << minimumSize();

	this->plotLayout()->setAlignCanvasToScales(true);

	QwtLegend *legend = new QwtLegend;
	legend->setDefaultItemMode(QwtLegendData::Clickable);
	this->insertLegend(legend, QwtPlot::BottomLegend);
	connect(legend, &QwtLegend::clicked, this, &Plot::on_legend_clicked);

	QwtPlotGrid *grid = new QwtPlotGrid();
	grid->setPen(Qt::gray, 0.0, Qt::DotLine);
	grid->enableX(true);
	grid->enableXMin(true);
	grid->enableY(true);
	grid->enableYMin(false);
	grid->attach(this);

	// Disable all x axis to have a known state for init_x_axis()
	this->enableAxis(QwtPlot::xBottom, false);
	this->enableAxis(QwtPlot::xTop, false);
	// Disable all y axis to have a known state for init_y_axis()
	this->enableAxis(QwtPlot::yLeft, false);
	this->enableAxis(QwtPlot::yRight, false);

	// Zooming and panning via the axes
	(void)new PlotScalePicker(this);

	// Panning via the canvas
	plot_panner_ = new QwtPlotPanner(this->canvas());
	connect(plot_panner_, &QwtPlotPanner::panned, this, &Plot::lock_all_axis);

	// Zooming via the canvas
	plot_magnifier_ = new PlotMagnifier(this->canvas());
	connect(plot_magnifier_, SIGNAL(magnified(double)),
		this, SLOT(lock_all_axis()));
}

Plot::~Plot()
{
	this->stop();
	for (const auto &marker_pair : marker_curve_map_)
		delete marker_pair.first;
	for (const auto &curve_pair : curve_map_)
		delete curve_pair.second;
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
	//qWarning() << "Plot::replot()";
	for (const auto &curve : curve_map_)
		curve.second->set_painted_points(0);

	QwtPlot::replot();
}

string Plot::add_curve(BaseCurveData *curve_data)
{
	assert(curve_data);

	// Check y axis
	int y_axis_id = this->init_y_axis(curve_data);
	if (y_axis_id < 0)
		return "";

	// Check x axis
	int x_axis_id = this->init_x_axis(curve_data);
	if (x_axis_id < 0)
		return "";

	Curve *curve = new Curve(curve_data, x_axis_id, y_axis_id);
	curve->plot_curve()->attach(this);
	curve_map_.insert(make_pair(curve->id(), curve));

	QwtPlot::replot();
	Q_EMIT curve_added();

	return curve->id();
}

bool Plot::add_curve(Curve *curve)
{
	if (curve_map_.count(curve->id()) > 0)
		return false;

	// Check y axis
	int y_axis_id = this->init_y_axis(curve->curve_data(), curve->y_axis_id());
	if (y_axis_id < 0)
		return false;

	// Check x axis
	int x_axis_id = this->init_x_axis(curve->curve_data(), curve->x_axis_id());
	if (x_axis_id < 0)
		return false;

	curve->plot_curve()->attach(this);
	curve_map_.insert(make_pair(curve->id(), curve));

	QwtPlot::replot();
	Q_EMIT curve_added();

	return true;
}

void Plot::remove_curve(Curve *curve)
{
	// Delete markers
	vector<QwtPlotMarker *> delete_markers;
	for (const auto &mc_pair : marker_curve_map_) {
		if (mc_pair.second == curve)
			delete_markers.push_back(mc_pair.first);
	}
	for (const auto &marker : delete_markers) {
		remove_marker(marker);
	}

	// Delete curve
	curve_map_.erase(curve->id());
	curve->plot_curve()->detach();
	delete curve;

	Q_EMIT curve_removed();
}

void Plot::remove_all_curves()
{
	for (const auto &curve_pair : curve_map_) {
		remove_curve(curve_pair.second);
	}
}

int Plot::init_x_axis(BaseCurveData *curve_data, int x_axis_id)
{
	assert(curve_data);

	if (x_axis_id < 0) {
		// Check if there already is an axis with the same unit. This is done
		// via the strings to get potential AC/DC flags.
		for (const auto &curve : curve_map_) {
			if (curve_data->x_unit_str() == curve.second->curve_data()->x_unit_str())
				return curve.second->x_axis_id();
		}
		// No existing axis was found, try to use the bottom or top axis.
		if (!this->axisEnabled(QwtPlot::xBottom))
			x_axis_id = QwtPlot::xBottom;
		else if (!this->axisEnabled(QwtPlot::xTop))
			x_axis_id = QwtPlot::xTop;
		else
			return -1;
	}
	else {
		// Check if the given axis id is already initialised with the proper
		// unit.
		for (const auto &curve : curve_map_) {
			if (curve.second->x_axis_id() == x_axis_id &&
					curve.second->curve_data()->x_unit_str() == curve_data->x_unit_str())
				return x_axis_id;
			else if (curve.second->x_axis_id() == x_axis_id) // NOLINT
				return -1;
		}
	}

	double min;
	double max;
	if (curve_data->type() == CurveType::TimeCurve &&
			curve_data->is_relative_time()) {
		min = 0.;
		max = add_time_;
	}
	else if (curve_data->type() == CurveType::TimeCurve &&
			!curve_data->is_relative_time()) {
		min = Session::session_start_timestamp;
		max = min + add_time_;
	}
	else if (curve_data->type() == CurveType::XYCurve) {
		// Values +/- 10%
		min = curve_data->boundingRect().left() -
			(std::fabs(curve_data->boundingRect().left()) * 0.1);
		max = curve_data->boundingRect().right() +
			(std::fabs(curve_data->boundingRect().right()) * 0.1);
	}
	else {
		throw std::runtime_error(
			"Plot::init_x_axis(): Curve type not implemented!");
	}

	this->init_axis(x_axis_id, min, max, curve_data->x_title(), true);

	if (curve_data->type() == CurveType::TimeCurve &&
			!curve_data->is_relative_time())
		this->setAxisScaleEngine(x_axis_id, new QwtDateScaleEngine());

	return x_axis_id;
}

int Plot::init_y_axis(BaseCurveData *curve_data, int y_axis_id)
{
	assert(curve_data);

	if (y_axis_id < 0) {
		// Check if there already is an axis with the same unit. This is done
		// via the strings to get potential AC/DC flags.
		for (const auto &curve : curve_map_) {
			if (curve_data->y_unit_str() == curve.second->curve_data()->y_unit_str())
				return curve.second->y_axis_id();
		}
		// No existing axis was found, try to use the left or right axis.
		if (!this->axisEnabled(QwtPlot::yLeft))
			y_axis_id = QwtPlot::yLeft;
		else if (!this->axisEnabled(QwtPlot::yRight))
			y_axis_id = QwtPlot::yRight;
		else
			return -1;
	}
	else {
		// Check if the given axis id is already initialised with the proper
		// unit.
		for (const auto &curve : curve_map_) {
			if (curve.second->y_axis_id() == y_axis_id &&
					curve.second->curve_data()->y_unit_str() == curve_data->y_unit_str())
				return y_axis_id;
			else if (curve.second->y_axis_id() == y_axis_id) // NOLINT
				return -1;
		}
	}

	// Values +/- 10%
	double min = curve_data->boundingRect().bottom() -
		(std::fabs(curve_data->boundingRect().bottom()) * 0.1);
	double max = curve_data->boundingRect().top() +
		(std::fabs(curve_data->boundingRect().top()) * 0.1);

	this->init_axis(y_axis_id, min, max, curve_data->y_title(), false);

	return y_axis_id;
}

void Plot::init_axis(int axis_id, double min, double max, const QString &title,
	bool auto_scale)
{
	map<AxisBoundary, bool> locks;
	locks.insert(make_pair<AxisBoundary, bool>(
		AxisBoundary::LowerBoundary, false));
	locks.insert(make_pair<AxisBoundary, bool>(
		AxisBoundary::UpperBoundary, false));
	axis_lock_map_.insert(make_pair(axis_id, locks));

	this->setAxisTitle(axis_id, title);
	this->setAxisScale(axis_id, min, max);
	this->setAxisAutoScale(axis_id, auto_scale); // TODO: Not working!?
	this->enableAxis(axis_id);
	this->add_axis_icons(axis_id);
}

void Plot::add_axis_icons(const int axis_id)
{
	AxisLockLabel *upper_lock_label = new AxisLockLabel(
		axis_id, AxisBoundary::UpperBoundary, "");
	connect(upper_lock_label, &AxisLockLabel::clicked,
		this, &Plot::on_axis_lock_clicked);
	connect(this, &Plot::axis_lock_changed,
		upper_lock_label, &AxisLockLabel::on_axis_lock_changed);

	AxisLockLabel *lower_lock_label = new AxisLockLabel(
		axis_id, AxisBoundary::LowerBoundary, "");
	connect(lower_lock_label, &AxisLockLabel::clicked,
		this, &Plot::on_axis_lock_clicked);
	connect(this, &Plot::axis_lock_changed,
		lower_lock_label, &AxisLockLabel::on_axis_lock_changed);

	QBoxLayout *scale_layout;
	if (axis_id == QwtPlot::xTop || axis_id == QwtPlot::xBottom) {
		scale_layout = new QHBoxLayout();
		if (axis_id == QwtPlot::xTop)
			scale_layout->setAlignment(Qt::AlignTop);
		else
			scale_layout->setAlignment(Qt::AlignBottom);
		scale_layout->addWidget(lower_lock_label);
		scale_layout->addStretch(1);
		scale_layout->addWidget(upper_lock_label);
		// NOTE: setContentsMargins() places the locks more nicely in Windows.
		scale_layout->setContentsMargins(0, 2, 0, 2);
	}
	else {
		scale_layout = new QVBoxLayout();
		if (axis_id == QwtPlot::yLeft)
			scale_layout->setAlignment(Qt::AlignLeft);
		else
			scale_layout->setAlignment(Qt::AlignRight);
		scale_layout->addWidget(upper_lock_label);
		scale_layout->addStretch(1);
		scale_layout->addWidget(lower_lock_label);
		// NOTE: setContentsMargins() places the locks more nicely in Windows.
		scale_layout->setContentsMargins(2, 0, 2, 0);
	}

	QwtScaleWidget *scale_widget = this->axisWidget(axis_id);
	scale_widget->setLayout(scale_layout);
}

void Plot::set_axis_locked(int axis_id, AxisBoundary axis_boundary, bool locked)
{
	axis_lock_map_[axis_id][axis_boundary] = locked;
	Q_EMIT axis_lock_changed(axis_id, axis_boundary, locked);
}

void Plot::set_all_axis_locked(bool locked)
{
	for (const auto &axis_lock : axis_lock_map_) {
		set_axis_locked(axis_lock.first, AxisBoundary::LowerBoundary, locked);
		set_axis_locked(axis_lock.first, AxisBoundary::UpperBoundary, locked);
	}
}

void Plot::lock_all_axis()
{
	this->set_all_axis_locked(true);
}

void Plot::on_axis_lock_clicked()
{
	AxisLockLabel *lock_label = qobject_cast<AxisLockLabel *>(sender());
	if (lock_label) {
		bool locked = is_axis_locked(
			lock_label->get_axis_id(), lock_label->get_axis_boundary());

		lock_label->set_locked(!locked);
		set_axis_locked(lock_label->get_axis_id(),
			lock_label->get_axis_boundary(), !locked);
	}
}

void Plot::set_time_span(double time_span)
{
	time_span_ = time_span;

	// time_span_ is used in rolling mode and oscilloscope mode. Find the
	// last/highest x value/timestamp and use it to calculate the new
	// x axis interval.
	double last_timestamp = .0;
	for (const auto &curve : curve_map_) {
		if (curve.second->curve_data()->boundingRect().right() > last_timestamp)
			last_timestamp = curve.second->curve_data()->boundingRect().right();
	}
	// max must be set to the last timestamp to keep the manual calculation
	// of the scale divs for oscilloscope mode working.
	double max = last_timestamp;
	double min = max - time_span_;
	this->setAxisScale(QwtPlot::xBottom, min, max);
	this->replot();
}

void Plot::add_marker(sv::ui::widgets::plot::Curve *curve)
{
	auto *marker = curve->add_marker(
		QString::number(marker_curve_map().size()+1));

	// Initial marker position is in the middle of the plot screen or
	// at the end of the curve.
	QwtInterval x_interval = this->axisInterval(curve->x_axis_id());
	double x_mid = (x_interval.minValue() + x_interval.maxValue()) / 2;
	QwtInterval y_interval = this->axisInterval(curve->y_axis_id());
	double y_mid = (y_interval.minValue() + y_interval.maxValue()) / 2;
	marker->setValue(
		curve->curve_data()->closest_point(QPointF(x_mid, y_mid), nullptr));

	// Attach to plot
	marker->attach(this);

	marker_curve_map_.insert(make_pair(marker, curve));
	active_marker_ = marker;

	// Add pickers for _all_ markers, no matter of the axis
	if (!marker_select_picker_) {
		// Use QwtPlot::xBottom and QwtPlot::yLeft as axis. We calculate the
		// canvas positions for the markers in on_marker_selected()
		marker_select_picker_ = new QwtPlotPicker(
			QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::NoRubberBand,
			QwtPicker::AlwaysOff, this->canvas());
		marker_select_picker_->setStateMachine(new QwtPickerClickPointMachine());
		connect(marker_select_picker_, QOverload<const QPointF&>::of(&QwtPlotPicker::selected),
			this, &Plot::on_marker_selected);
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
	/*
	 * TODO: Maybe we could use a QwtPickerTrackerMachine for mouse movement.
	 * This way we can avoid the mouse click event (problems with QwtPlotPanner)
	 * and also highlight the marker that in the "selectable range" of the
	 * mouse pointer. Maybe this would be a performance issue?
	 */

	update_markers_label();
	replot();
	Q_EMIT marker_added();
}

void Plot::add_diff_marker(QwtPlotMarker *marker1, QwtPlotMarker *marker2)
{
	if (!marker1 || !marker2)
		return;

	diff_markers_.push_back(make_pair(marker1, marker2));

	update_markers_label();
	replot();
	Q_EMIT marker_added();
}

// TODO: Implement GUI remove marker function
void Plot::remove_marker(QwtPlotMarker *marker)
{
	// Delete diff markers.
	for (auto it = diff_markers_.begin(); it != diff_markers_.end(); ) {
		if (it->first == marker || it->second == marker)
			it = diff_markers_.erase(it);
		else
			++it;
	}

	// Delete marker.
	marker_curve_map_.erase(marker);
	if (active_marker_ == marker)
		active_marker_ = nullptr;
	marker->detach();
	delete marker;

	if (marker_curve_map_.empty()) {
		// No markers left.
		disconnect(marker_select_picker_, SIGNAL(selected(const QPointF &)),
			this, SLOT(on_marker_selected(const QPointF)));
		delete marker_select_picker_;
		marker_select_picker_ = nullptr;
		disconnect(marker_move_picker_, SIGNAL(moved(QPointF)),
			this, SLOT(on_marker_moved(QPointF)));
		delete marker_move_picker_;
		marker_move_picker_ = nullptr;
	}

	update_markers_label();
	replot();
	Q_EMIT marker_removed();
}

void Plot::on_marker_selected(const QPointF &mouse_pos)
{
	if (marker_curve_map_.empty())
		return;

	// Mouse canvas coordinates. Use QwtPlot::xBottom and QwtPlot::yLeft
	// as axis for marker_select_picker_.
	const double mouse_canvas_x = transform(QwtPlot::xBottom, mouse_pos.x());
	const double mouse_canvas_y = transform(QwtPlot::yLeft, mouse_pos.y());

	// Check if mouse pointer is near a marker
	double d_min = 15.; // Minimum distance to marker for selecting
	double d_lowest = 1.0e10;
	QwtPlotMarker *selected_marker = nullptr;
	for (const auto &mc_pair : marker_curve_map_) {
		// Marker canvas coordinates. Use axis ids form plot.
		const double marker_canvas_x =
			transform(mc_pair.second->x_axis_id(), mc_pair.first->xValue());
		const double marker_canvas_y =
			transform(mc_pair.second->y_axis_id(), mc_pair.first->yValue());

		const double d_x = marker_canvas_x - mouse_canvas_x;
		const double d_y = marker_canvas_y - mouse_canvas_y;
		const double d_actual = qSqrt(qwtSqr(d_x) + qwtSqr(d_y));
		if (d_actual <= d_min && d_actual < d_lowest) {
			d_lowest = d_actual;
			selected_marker = mc_pair.first;
		}
	}
	if (selected_marker) {
		plot_panner_->setEnabled(false);
		active_marker_ = selected_marker;
	}
	else {
		/*
		 * TODO: Maybe activate the plot panner via a timer after 1s of no
		 * marker move event. This would avoid the "double" click to deselect
		 * the marker/enable the panner (1) and register a new panning event (2)
		 */
		plot_panner_->setEnabled(true);
		active_marker_ = nullptr;
	}

	// TODO: connect/disconnect marker_move_picker_
}

void Plot::on_marker_moved(const QPointF mouse_pos)
{
	if (!active_marker_)
		return;

	BaseCurveData *curve_data = marker_curve_map_[active_marker_]->curve_data();
	QPointF marker_pos = curve_data->closest_point(mouse_pos, nullptr);
	active_marker_->setValue(marker_pos);

	update_markers_label();
	replot();
}

void Plot::on_legend_clicked(const QVariant &item_info, int index)
{
	(void)index;

	QwtPlotItem *plot_item = infoToItem(item_info);
	if (!plot_item)
		return;
	Curve *curve = get_curve_from_plot_curve((QwtPlotCurve *)plot_item);
	if (!curve)
		return;

	ui::dialogs::PlotCurveConfigDialog dlg(curve, this);
	dlg.exec();
}

void Plot::update_curves()
{
	for (const auto &curve : curve_map_) {
		const size_t painted_points = curve.second->painted_points();
		const size_t num_points = curve.second->curve_data()->size();
		if (num_points > painted_points) {
			//qWarning() << QString("Plot::updateCurve(): num_points = %1, painted_points = %2").
			//	arg(num_points).arg(painted_points);
			const bool clip = !canvas()->testAttribute(Qt::WA_PaintOnScreen);
			if (clip) {
				/*
				 * NOTE:
				 * Depending on the platform setting a clip might be an
				 * important performance issue. F.e. for Qt Embedded this
				 * reduces the part of the backing store that has to be copied
				 * out - maybe to an unaccelerated frame buffer device.
				 */

				const QwtScaleMap x_map = canvasMap(curve.second->x_axis_id());
				const QwtScaleMap y_map = canvasMap(curve.second->y_axis_id());
				QRectF br = qwtBoundingRect(*curve.second->plot_curve()->data(),
					(int)painted_points - 1, (int)num_points - 1);

				curve.second->plot_direct_painter()->setClipRegion(
					QwtScaleMap::transform(x_map, y_map, br).toRect());
			}
			curve.second->plot_direct_painter()->drawSeries(
				curve.second->plot_curve(), (int)painted_points - 1,
				(int)num_points - 1);
			curve.second->set_painted_points(num_points);
		}

		//replot();
	}
}

void Plot::update_intervals()
{
	bool intervals_changed = false;

	for (const auto &curve : curve_map_) {
		if (update_x_interval(curve.second))
			intervals_changed = true;
		if (update_y_interval(curve.second))
			intervals_changed = true;
	}

	if (intervals_changed)
		replot();
}

bool Plot::update_x_interval(Curve *curve)
{
	if (axis_lock_map_[QwtPlot::xBottom][AxisBoundary::LowerBoundary] &&
		axis_lock_map_[QwtPlot::xBottom][AxisBoundary::UpperBoundary])
		return false;

	bool interval_changed = false;
	QRectF boundaries = curve->curve_data()->boundingRect();
	QwtInterval x_interval = this->axisInterval(QwtPlot::xBottom);
	double min = x_interval.minValue();
	double max = x_interval.maxValue();

	// There are no plot modes when showing xy curves, just extend the intervals
	if (curve->curve_data()->type() == CurveType::XYCurve) {
		if (!axis_lock_map_[QwtPlot::xBottom][AxisBoundary::LowerBoundary] &&
				boundaries.left() < min) {
			// New value - 10%
			min = boundaries.left() - (std::fabs(boundaries.left()) * 0.1);
			interval_changed = true;
		}
		if (!axis_lock_map_[QwtPlot::xBottom][AxisBoundary::UpperBoundary] &&
				boundaries.right() > max) {
			// New value + 10%
			max = boundaries.right() + (std::fabs(boundaries.right()) * 0.1);
			interval_changed = true;
		}

		if (interval_changed)
			setAxisScale(QwtPlot::xBottom, min, max);
	}
	// Handle the Additive plot mode
	else if (update_mode_ == PlotUpdateMode::Additive) {
		if (!axis_lock_map_[QwtPlot::xBottom][AxisBoundary::LowerBoundary] &&
				boundaries.left() < min) {
			min = 0;
			interval_changed = true;
		}
		if (!axis_lock_map_[QwtPlot::xBottom][AxisBoundary::UpperBoundary] &&
				boundaries.right() > max) {
			if (boundaries.right()+add_time_ > max)
				max = boundaries.right() + add_time_;
			else
				max = x_interval.maxValue() + add_time_;
			interval_changed = true;
		}

		if (interval_changed)
			setAxisScale(QwtPlot::xBottom, min, max);
	}
	// Handle the Rolling plot mode
	else if (update_mode_ == PlotUpdateMode::Rolling) {
		// TODO: axis locking. Lock/Unlock both upper and lower together!
		if (boundaries.right() <= max)
			return false;

		if (boundaries.right() > max+time_span_)
			min = boundaries.right();
		else
			min += add_time_;
		max = min + time_span_;

		interval_changed = true;
		setAxisScale(QwtPlot::xBottom, min, max);
	}
	// Handle the Oscilloscope plot mode
	else if (update_mode_ == PlotUpdateMode::Oscilloscope) {
		// TODO: axis locking. Lock/Unlock both upper and lower together?
		if (boundaries.right() <= max)
			return false;

		if (boundaries.right() > max+time_span_)
			min = boundaries.right();
		else
			min += time_span_;
		max = min + time_span_;

		/*
		 * NOTE:
		 * To avoid, that the grid is jumping, we disable the autocalculation
		 * of the ticks and shift them manually instead.
		 */
		QwtScaleDiv scaleDiv = axisScaleDiv(QwtPlot::xBottom);
		scaleDiv.setInterval(min, max);
		for (int i = 0; i < QwtScaleDiv::NTickTypes; i++) {
			QList<double> ticks = scaleDiv.ticks(i);
			for (int j = 0; j < ticks.size(); j++) {
				ticks[j] += x_interval.width();
			}
			scaleDiv.setTicks(i, ticks);
		}

		interval_changed = true;
		setAxisScaleDiv(QwtPlot::xBottom, scaleDiv);
		curve->set_painted_points(0);
	}

	return interval_changed;
}

bool Plot::update_y_interval(const Curve *curve)
{
	int y_axis_id = curve->y_axis_id();
	if (axis_lock_map_[y_axis_id][AxisBoundary::LowerBoundary] &&
			axis_lock_map_[y_axis_id][AxisBoundary::UpperBoundary])
		return false;

	QRectF boundaries = curve->curve_data()->boundingRect();
	QwtInterval y_interval = this->axisInterval(y_axis_id);
	double min = y_interval.minValue();
	double max = y_interval.maxValue();
	bool interval_changed = false;

	if (!axis_lock_map_[y_axis_id][AxisBoundary::LowerBoundary] &&
			boundaries.bottom() < min) {
		// New value - 10%
		min = boundaries.bottom() - (std::fabs(boundaries.bottom()) * 0.1);
		interval_changed = true;
	}
	if (!axis_lock_map_[y_axis_id][AxisBoundary::UpperBoundary] &&
			boundaries.top() > max) {
		// New value + 10%
		max = boundaries.top() + (std::fabs(boundaries.top()) * 0.1);
		interval_changed = true;
	}

	if (interval_changed )
		setAxisScale(y_axis_id, min, max);
	return interval_changed;
}

void Plot::set_markers_label_alignment(int alignment)
{
	markers_label_alignment_ = alignment;
	if (markers_label_) {
		// TODO: Maybe there is a better way to replot the label?
		markers_label_->detach();
		delete markers_label_;
		markers_label_ = nullptr;
		update_markers_label();
	}
}

void Plot::update_markers_label()
{
	if (!markers_label_) {
		markers_label_ = new QwtPlotTextLabel();
		markers_label_->setMargin(5);
		// The markers label will be painted ontop of curves.
		markers_label_->setZ(3);
		markers_label_->attach(this);
	}

	QString table("<table>");

	for (const auto &mc_pair : marker_curve_map_) {
		table.append("<tr>");
		table.append(QString("<td width=\"50\" align=\"left\">%1:</td>").
			arg(mc_pair.first->title().text()));
		table.append(QString("<td width=\"70\" align=\"right\">%2 %3</td>").
			arg(mc_pair.first->yValue()).
			arg(mc_pair.second->curve_data()->y_unit_str()));
		table.append(QString("<td width=\"70\" align=\"right\">%4 %5</td>").
			arg(mc_pair.first->xValue()).
			arg(mc_pair.second->curve_data()->x_unit_str()));
		table.append("</tr>");
	}

	for (const auto &marker_pair : diff_markers_) {
		double d_x = marker_pair.first->xValue() - marker_pair.second->xValue();
		double d_y = marker_pair.first->yValue() - marker_pair.second->yValue();

		QString x_unit("");
		QString m1_x_unit =
			marker_curve_map_[marker_pair.first]->curve_data()->x_unit_str();
		QString m2_x_unit =
			marker_curve_map_[marker_pair.second]->curve_data()->x_unit_str();
		if (m1_x_unit == m2_x_unit)
			x_unit = m1_x_unit;

		QString y_unit("");
		QString m1_y_unit =
			marker_curve_map_[marker_pair.first]->curve_data()->y_unit_str();
		QString m2_y_unit =
			marker_curve_map_[marker_pair.second]->curve_data()->y_unit_str();
		if (m1_y_unit == m2_y_unit)
			y_unit = m1_y_unit;

		table.append("<tr>");
		table.append(QString("<td width=\"50\" align=\"left\">%1 - %2:</td>").arg(
			marker_pair.first->title().text(),
			marker_pair.second->title().text()));
		table.append(QString("<td width=\"70\" align=\"right\">%1 %2</td>").
			arg(d_y).arg(y_unit));
		table.append(QString("<td width=\"70\" align=\"right\">%1 %2</td>").
			arg(d_x).arg(x_unit));
		table.append("</tr>");
	}

	table.append("</table>");

	QwtText text = QwtText(table);
	text.setPaintAttribute(QwtText::PaintBackground, true);
	QColor background(Qt::gray);
	background.setAlpha(200);
	text.setBackgroundBrush(background);
	text.setBorderRadius(3);
	QPen pen;
	pen.setColor(Qt::black);
	pen.setWidthF(1.0);
	pen.setStyle(Qt::SolidLine);
	text.setBorderPen(pen);
	text.setRenderFlags(markers_label_alignment_);
	markers_label_->setText(text);
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
	for (const auto &curve : curve_map_) {
		curve.second->plot_direct_painter()->reset();
	}

	QwtPlot::resizeEvent(event);
}

void Plot::showEvent(QShowEvent *event)
{
	(void)event;
	replot();
}

bool Plot::eventFilter(QObject *object, QEvent *event)
{
	return QwtPlot::eventFilter(object, event);
}

void Plot::save_settings(QSettings &settings, bool save_curves,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	// TODO: Use Q_ENUM_NS for PlotUpdateMode to add meta-object support (Qt >= 5.8)
	settings.setValue("update_mode", (int)update_mode());
	settings.setValue("time_span", time_span_);
	settings.setValue("add_time", add_time_);

	if (!save_curves)
		return;
	for (const auto &curve : curve_map_) {
		curve.second->save_settings(settings, origin_device);
	}
}

void Plot::restore_settings(QSettings &settings, bool restore_curves,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	if (settings.contains("update_mode"))
		update_mode_ = static_cast<PlotUpdateMode>(
			settings.value("update_mode").toInt());
	if (settings.contains("time_span"))
		time_span_ = settings.value("time_span").toDouble();
	if (settings.contains("add_time"))
		add_time_ = settings.value("add_time").toDouble();

	if (!restore_curves)
		return;
	const auto groups = settings.childGroups();
	for (const auto &group : groups) {
		if (group.startsWith("timecurve:") || group.startsWith("xycurve:")) {
			Curve *curve = Curve::init_from_settings(
				session_, settings, group, origin_device);
			if (curve)
				add_curve(curve);
		}
	}
}

Curve *Plot::get_curve_from_plot_curve(const QwtPlotCurve *plot_curve) const
{
	for (const auto &curve : curve_map_) {
		if (curve.second->plot_curve() == plot_curve)
			return curve.second;
	}
	return nullptr;
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
