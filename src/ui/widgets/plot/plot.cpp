/*
 * This file is part of the SmuView project.
 * This file is based on the QWT Oscilloscope Example.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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
#include <utility>

#include <QBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
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

#include "plot.hpp"
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
	Canvas(QwtPlot *plot = nullptr) : QwtPlotCanvas(plot)
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
	// indirectly in a (Main)Window, the minimum size is way to big
	this->setMinimumSize(250, 250);
	//qWarning() << "Plot::Plot(): sizeHint() = " << sizeHint() <<
	//	", minimumSizeHint() = " << minimumSizeHint();
	//qWarning() << "Plot::Plot(): size() = " << size() <<
	//	", minimumSize() = " << minimumSize();

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

	// Zooming and panning via the axes
	(void)new PlotScalePicker(this);

	// Panning via the canvas
	plot_panner_ = new QwtPlotPanner(this->canvas());
	connect(plot_panner_, SIGNAL(panned(int, int)),
		this, SLOT(lock_all_axis()));

	// Zooming via the canvas
	plot_magnifier_ = new PlotMagnifier(this->canvas());
	connect(plot_magnifier_, SIGNAL(magnified(double)),
		this, SLOT(lock_all_axis()));
}

Plot::~Plot()
{
	//qWarning() << "Plot::~Plot() for " << curve_data_->name();
	this->stop();
	for (auto &direct_painter_pair : plot_direct_painter_map_) {
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
	//qWarning() << "Plot::replot()";

	for (const auto &curve_data : curve_datas_) {
		painted_points_map_[curve_data] = 0;
	}

	QwtPlot::replot();
}

bool Plot::add_curve(widgets::plot::BaseCurveData *curve_data)
{
	assert(curve_data);

	// Check if curve already has been added
	for (const auto &cd : curve_datas_) {
		if (cd->is_equal(curve_data))
			return true;
	}

	// Check y axis
	int y_axis_id = this->init_y_axis(curve_data);
	if (y_axis_id < 0)
		return false;

	// Check x axis
	int x_axis_id = this->init_x_axis(curve_data);
	if (x_axis_id < 0)
		return false;

	curve_datas_.push_back(curve_data);

	QPen pen;
	pen.setColor(curve_data->color());
	pen.setWidthF(2.0);
	pen.setStyle(Qt::SolidLine);
	pen.setCosmetic(false);

	// Set empty symbol, used in the PlotCurveConfigDialog.
	QwtSymbol *symbol = new QwtSymbol(QwtSymbol::NoSymbol);

	QwtPlotCurve *plot_curve = new QwtPlotCurve(curve_data->name());
	plot_curve->setYAxis(y_axis_id);
	plot_curve->setXAxis(x_axis_id);
	plot_curve->setStyle(QwtPlotCurve::Lines);
	plot_curve->setPen(pen);
	plot_curve->setSymbol(symbol);
	plot_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	plot_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
	plot_curve->setData(curve_data);
	//plot_curve->setRawSamples(); // TODO: is this an option?
	// Curves have the lowest z order, everything else will be painted ontop.
	plot_curve->setZ(1);
	plot_curve->attach(this);
	plot_curve_map_.insert(make_pair(curve_data, plot_curve));

	QwtPlotDirectPainter *direct_painter = new QwtPlotDirectPainter();
	plot_direct_painter_map_.insert(make_pair(curve_data, direct_painter));

	painted_points_map_.insert(make_pair(curve_data, 0));

	QwtPlot::replot();

	return true;
}

int Plot::init_x_axis(widgets::plot::BaseCurveData *curve_data)
{
	assert(curve_data);

	int x_axis_id = QwtPlot::xBottom;

	// Check if there already is an axis with the same unit. This is done
	// via the strings to get potential AC/DC flags.
	if (!curve_datas_.empty()) {
		if (curve_data->x_unit_str() != curve_datas_[0]->x_unit_str())
			return -1;
		return x_axis_id;
	}

	QString title = curve_data->x_title();
	double min;
	double max;
	if (curve_data->curve_type() == CurveType::TimeCurve &&
			curve_data->is_relative_time()) {
		min = 0.;
		max = add_time_;
		// TODO: !curve_data->is_relative_time()
	}
	else if (curve_data->curve_type() == CurveType::XYCurve) {
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

	map<AxisBoundary, bool> locks;
	locks.insert(make_pair<AxisBoundary, bool>(
		AxisBoundary::LowerBoundary, false));
	locks.insert(make_pair<AxisBoundary, bool>(
		AxisBoundary::UpperBoundary, false));
	axis_lock_map_.insert(make_pair(x_axis_id, locks));

	this->setAxisTitle(x_axis_id, title);
	this->setAxisScale(x_axis_id, min, max);
	//this->setAxisAutoScale(x_axis_id, true); // TODO: Not working!?
	this->enableAxis(x_axis_id);
	this->add_axis_icons(x_axis_id);

	if (curve_data->curve_type() == CurveType::TimeCurve &&
			!curve_data->is_relative_time())
		this->setAxisScaleEngine(x_axis_id, new QwtDateScaleEngine());

	return x_axis_id;
}

int Plot::init_y_axis(widgets::plot::BaseCurveData *curve_data)
{
	assert(curve_data);

	bool do_init = false;
	int y_axis_id = -1;
	if (!this->axisEnabled(QwtPlot::yLeft)) {
		y_axis_id = QwtPlot::yLeft;
		do_init = true;
	}
	else {
		// Check if there already is an axis with the same unit. This is done
		// via the strings to get potential AC/DC flags.
		for (const auto &cid_pair : y_axis_id_map_) {
			if (cid_pair.first->y_unit_str() == curve_data->y_unit_str()) {
				y_axis_id = cid_pair.second;
				do_init = false;
			}
		}
	}
	if (y_axis_id < 0) {
		if (!this->axisEnabled(QwtPlot::yRight)) {
			y_axis_id = QwtPlot::yRight;
			do_init = true;
		}
		else
			return y_axis_id;
	}

	y_axis_id_map_.insert(make_pair(curve_data, y_axis_id));

	if (do_init) {
		map<AxisBoundary, bool> locks;
		locks.insert(make_pair<AxisBoundary, bool>(
			AxisBoundary::LowerBoundary, false));
		locks.insert(make_pair<AxisBoundary, bool>(
			AxisBoundary::UpperBoundary, false));
		axis_lock_map_.insert(make_pair(y_axis_id, locks));

		// Values +/- 10%
		double min = curve_data->boundingRect().bottom() -
			(std::fabs(curve_data->boundingRect().bottom()) * 0.1);
		double max = curve_data->boundingRect().top() +
			(std::fabs(curve_data->boundingRect().top()) * 0.1);

		this->setAxisTitle(y_axis_id, curve_data->y_title());
		this->setAxisScale(y_axis_id, min, max);
		this->setAxisAutoScale(y_axis_id, false); // TODO: Not working!?
		this->enableAxis(y_axis_id);
		this->add_axis_icons(y_axis_id);
	}

	return y_axis_id;
}

void Plot::add_axis_icons(const int axis_id)
{
	AxisLockLabel *upper_lock_label = new AxisLockLabel(
		axis_id, AxisBoundary::UpperBoundary, "");
	connect(upper_lock_label, SIGNAL(clicked()),
		this, SLOT(on_axis_lock_clicked()));
	connect(this, SIGNAL(axis_lock_changed(int, AxisBoundary, bool)),
		upper_lock_label,
		SLOT(on_axis_lock_changed(const int, const AxisBoundary, bool)));

	AxisLockLabel *lower_lock_label = new AxisLockLabel(
		axis_id, AxisBoundary::LowerBoundary, "");
	connect(lower_lock_label, SIGNAL(clicked()),
		this, SLOT(on_axis_lock_clicked()));
	connect(this, SIGNAL(axis_lock_changed(int, AxisBoundary, bool)),
		lower_lock_label,
		SLOT(on_axis_lock_changed(const int, const AxisBoundary, bool)));

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
	for (const auto &p : axis_lock_map_) {
		set_axis_locked(p.first, AxisBoundary::LowerBoundary, locked);
		set_axis_locked(p.first, AxisBoundary::UpperBoundary, locked);
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
	for (const auto &curve : curve_datas_) {
		if (curve->boundingRect().right() > last_timestamp)
			last_timestamp = curve->boundingRect().right();
	}
	// max must be set to the last timestamp to keep the manual calculation
	// of the scale divs for oscilloscope mode working.
	double max = last_timestamp;
	double min = max - time_span_;
	this->setAxisScale(QwtPlot::xBottom, min, max);
	this->replot();
}

void Plot::add_marker(plot::BaseCurveData *curve_data)
{
	assert(curve_data);

	QwtPlotCurve *plot_curve = plot_curve_map_[curve_data];

	QwtSymbol *marker_sym = new QwtSymbol(
		QwtSymbol::Diamond, QBrush(Qt::red), QPen(Qt::red), QSize(9, 9));
	QString marker_name = QString("M%1").arg(markers_.size()+1);

	QwtPlotMarker *marker = new QwtPlotMarker(marker_name);
	marker->setSymbol(marker_sym);
	marker->setLineStyle(QwtPlotMarker::Cross);
	marker->setLinePen(Qt::white, 1.0, Qt::DashLine);
	marker->setXAxis(plot_curve->xAxis());
	marker->setYAxis(plot_curve->yAxis());
	// Markers will be painted ontop of curves but below the markers label box.
	marker->setZ(2);

	// Initial marker position is in the middle of the plot screen or
	// at the end of the curve.
	QwtInterval x_interval = this->axisInterval(plot_curve->xAxis());
	double x_mid = (x_interval.minValue() + x_interval.maxValue()) / 2;
	QwtInterval y_interval = this->axisInterval(plot_curve->yAxis());
	double y_mid = (y_interval.minValue() + y_interval.maxValue()) / 2;
	marker->setValue(curve_data->closest_point(QPointF(x_mid, y_mid), nullptr));

	// Label
	QwtText marker_label = QwtText(marker_name);
	marker_label.setColor(Qt::black);
	marker_label.setPaintAttribute(QwtText::PaintBackground, true);
	QColor c(Qt::gray);
	c.setAlpha(200);
	marker_label.setBackgroundBrush(c);
	QPen pen(Qt::black, 1.0, Qt::SolidLine);
	marker_label.setBorderPen(pen);
	marker_label.setBorderRadius(3);
	marker->setLabel(marker_label);
	marker->setLabelAlignment(Qt::AlignTop | Qt::AlignRight);

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
	/*
	 * TODO: Maybe we could use a QwtPickerTrackerMachine for mouse movement.
	 * This way we can avoid the mouse click event (problems with QwtPlotPanner)
	 * and also highlight the marker that in the "selectable range" of the
	 * mouse pointer. Maybe this would be a performance issue?
	 */

	update_markers_label();
	replot();
}

void Plot::add_diff_marker(QwtPlotMarker *marker1, QwtPlotMarker *marker2)
{
	if (!marker1 || !marker2)
		return;

	diff_markers_.push_back(make_pair(marker1, marker2));

	update_markers_label();
	replot();
}

// TODO: implement remove marker call
void Plot::remove_marker()
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

void Plot::on_marker_selected(const QPointF mouse_pos)
{
	if (markers_.empty())
		return;

	// Mouse canvas coordinates. Use QwtPlot::xBottom and QwtPlot::yLeft
	// as axis for marker_select_picker_.
	const double mouse_canvas_x = transform(QwtPlot::xBottom, mouse_pos.x());
	const double mouse_canvas_y = transform(QwtPlot::yLeft, mouse_pos.y());

	// Check if mouse pointer is near a marker
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

	plot::BaseCurveData *curve_data = marker_map_[active_marker_];
	QPointF marker_pos = curve_data->closest_point(mouse_pos, nullptr);
	active_marker_->setValue(marker_pos);

	update_markers_label();
	replot();
}

void Plot::on_legend_clicked(const QVariant &item_info, int index)
{
	(void)index;

	QwtPlotItem *plot_item = infoToItem(item_info);
	if (plot_item) {
		QwtPlotCurve *plot_curve = (QwtPlotCurve *)plot_item;
		ui::dialogs::PlotCurveConfigDialog dlg(plot_curve);
		dlg.exec();
	}
}

void Plot::update_curves()
{
	for (const auto &curve_data : curve_datas_) {
		const size_t painted_points = painted_points_map_[curve_data];
		const size_t num_points = curve_data->size();
		if (num_points > painted_points) {
			QwtPlotCurve *plot_curve = plot_curve_map_[curve_data];
			QwtPlotDirectPainter *direct_painter =
				plot_direct_painter_map_[curve_data];

			//qWarning() << QString("Plot::updateCurve(): num_points = %1, painted_points = %2").arg(num_points).arg(painted_points);
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
					(int)painted_points - 1, (int)num_points - 1);
				const QRect clip_rect =
					QwtScaleMap::transform(x_map, y_map, br).toRect();

				direct_painter->setClipRegion(clip_rect);
			}

			direct_painter->drawSeries(plot_curve,
				(int)painted_points - 1, (int)num_points - 1);

			painted_points_map_[curve_data] = num_points;
		}

		//replot();
	}
}

void Plot::update_intervals()
{
	bool intervals_changed = false;

	for (const auto &curve_data : curve_datas_) {
		if (update_x_interval(curve_data))
			intervals_changed = true;
		if (update_y_interval(curve_data))
			intervals_changed = true;
	}

	if (intervals_changed)
		replot();
}

bool Plot::update_x_interval(plot::BaseCurveData *curve_data)
{
	if (axis_lock_map_[QwtPlot::xBottom][AxisBoundary::LowerBoundary] &&
		axis_lock_map_[QwtPlot::xBottom][AxisBoundary::UpperBoundary])
		return false;

	bool interval_changed = false;
	QRectF boundaries = curve_data->boundingRect();
	QwtInterval x_interval = this->axisInterval(QwtPlot::xBottom);
	double min = x_interval.minValue();
	double max = x_interval.maxValue();

	// There are no plot modes when showing xy curves, just extend the intervals
	if (curve_data->curve_type() == CurveType::XYCurve) {
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
		painted_points_map_[curve_data] = 0;
	}

	return interval_changed;
}

bool Plot::update_y_interval(plot::BaseCurveData *curve_data)
{
	int y_axis_id = y_axis_id_map_[curve_data];
	if (axis_lock_map_[y_axis_id][AxisBoundary::LowerBoundary] &&
			axis_lock_map_[y_axis_id][AxisBoundary::UpperBoundary])
		return false;

	QRectF boundaries = curve_data->boundingRect();
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

	for (const auto &marker : markers_) {
		table.append("<tr>");
		table.append(QString("<td width=\"50\" align=\"left\">%1:</td>").
			arg(marker->title().text()));
		table.append(QString("<td width=\"70\" align=\"right\">%2 %3</td>").
			arg(marker->yValue()).
			arg(marker_map_[marker]->y_unit_str()));
		table.append(QString("<td width=\"70\" align=\"right\">%4 %5</td>").
			arg(marker->xValue()).
			arg(marker_map_[marker]->x_unit_str()));
		table.append("</tr>");
	}

	for (const auto &marker_pair : diff_markers_) {
		double d_x = marker_pair.first->xValue() - marker_pair.second->xValue();
		double d_y = marker_pair.first->yValue() - marker_pair.second->yValue();

		QString x_unit("");
		QString m1_x_unit = marker_map_[marker_pair.first]->x_unit_str();
		QString m2_x_unit = marker_map_[marker_pair.second]->x_unit_str();
		if (m1_x_unit == m2_x_unit)
			x_unit = m1_x_unit;

		QString y_unit("");
		QString m1_y_unit = marker_map_[marker_pair.first]->y_unit_str();
		QString m2_y_unit = marker_map_[marker_pair.second]->y_unit_str();
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
	for (auto &direct_painter_pair : plot_direct_painter_map_) {
		direct_painter_pair.second->reset();
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

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
