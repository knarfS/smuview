/*
 * This file is part of the SmuView project.
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

#include <cassert>
#include <memory>
#include <regex>

#include <QColor>
#include <QDebug>
#include <QObject>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif
#include <QRectF>
#include <QPen>
#include <QSettings>
#include <QUuid>
#include <QVariant>
#include <qwt_curve_fitter.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_div.h>
#include <qwt_scale_map.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>

#include "scopecurve.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/util.hpp"
#include "src/data/analogscopesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"
#include "src/ui/widgets/plot/plotcanvas.hpp"
#include "src/ui/widgets/plot/scopecurvedata.hpp"
#include "src/ui/widgets/plot/timecurvedata.hpp"
#include "src/ui/widgets/plot/xycurvedata.hpp"

using std::dynamic_pointer_cast;
using std::shared_ptr;

Q_DECLARE_METATYPE(QwtPlot::Axis)
Q_DECLARE_METATYPE(QwtSymbol::Style)

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

ScopeCurve::ScopeCurve(shared_ptr<sv::data::AnalogScopeSignal> signal,
		QwtPlot::Axis x_axis_id, QwtPlot::Axis y_axis_id) :
	signal_(signal),
	plot_direct_painter_(new QwtPlotDirectPainter()),
	type_(CurveType::ScopeCurve),
	relative_time_(true),
	painted_points_(0)
{
	id_ = "scopecurve:" + util::format_uuid(QUuid::createUuid());

	QPen pen;
	pen.setColor(ScopeCurve::default_color(signal_->name()));
	pen.setWidthF(2.0);
	pen.setStyle(Qt::SolidLine);
	pen.setCosmetic(false);

	plot_curve_ = new QwtPlotCurve(name());
	plot_curve_->setStyle(QwtPlotCurve::Lines);
	plot_curve_->setPen(pen);
	// Set empty symbol, used in the PlotCurveConfigDialog.
	plot_curve_->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol));
	plot_curve_->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	plot_curve_->setPaintAttribute(QwtPlotCurve::ClipPolygons, true);
	plot_curve_->setPaintAttribute(QwtPlotCurve::FilterPoints, true);
	// Curves have the lowest z order, everything else will be painted ontop.
	plot_curve_->setZ(1);
	// Set axes
	plot_curve_->setXAxis(x_axis_id);
	plot_curve_->setYAxis(y_axis_id);

	/* TODO: Add/Get all existing segments? */
	if (signal_->get_last_segment() != nullptr)
		new_segment(signal_->get_last_segment()->id());

	/* Old connects from (Scope)Plot */
	// TODO: works somehow, more or less without the reset....
	//connect(curve, &Curve::reset,
	//	this, [this]() { replot(); });

	connect(signal_.get(), &sv::data::AnalogScopeSignal::segment_added,
		this, &ScopeCurve::new_segment);
	connect(signal_.get(), &sv::data::AnalogScopeSignal::samples_added,
		this, &ScopeCurve::update);
}

ScopeCurve::~ScopeCurve()
{
	delete plot_curve_;
	delete plot_direct_painter_;
}


CurveType ScopeCurve::type() const
{
	return type_;
}

string ScopeCurve::id() const
{
	return id_;
}

void ScopeCurve::set_name(const QString &custom_name)
{
	if (custom_name.size() > 0) {
		has_custom_name_ = true;
		name_ = custom_name;
	}
	else {
		has_custom_name_ = false;
		name_ = name();
	}
	plot_curve_->setTitle(name_);
}

QString ScopeCurve::name() const
{
	return signal_->display_name();
}

string ScopeCurve::id_prefix() const
{
	return "scopecurve";
}

void ScopeCurve::set_relative_time(bool is_relative_time)
{
	relative_time_ = is_relative_time;
}

bool ScopeCurve::is_relative_time() const
{
	return relative_time_;
}

QwtPlot::Axis ScopeCurve::x_axis_id() const
{
	return static_cast<QwtPlot::Axis>(plot_curve_->xAxis());
}

QwtPlot::Axis ScopeCurve::y_axis_id() const
{
	return static_cast<QwtPlot::Axis>(plot_curve_->yAxis());
}

sv::data::Quantity ScopeCurve::x_quantity() const
{
	return sv::data::Quantity::Time;
}

set<sv::data::QuantityFlag> ScopeCurve::x_quantity_flags() const
{
	return set<data::QuantityFlag>();
}

sv::data::Unit ScopeCurve::x_unit() const
{
	return sv::data::Unit::Second;
}

QString ScopeCurve::x_unit_str() const
{
	return data::datautil::format_unit(x_unit());
}

QString ScopeCurve::x_title() const
{
	return QString("%1 [%2]").
		arg(data::datautil::format_quantity(x_quantity()), x_unit_str());
}

sv::data::Quantity ScopeCurve::y_quantity() const
{
	return signal_->quantity();
}

set<sv::data::QuantityFlag> ScopeCurve::y_quantity_flags() const
{
	return signal_->quantity_flags();
}

sv::data::Unit ScopeCurve::y_unit() const
{
	return signal_->unit();
}

QString ScopeCurve::y_unit_str() const
{
	return data::datautil::format_unit(y_unit(), y_quantity_flags());
}

QString ScopeCurve::y_title() const
{
	// Don't use only the unit, so we can add AC/DC to axis label.
	return QString("%1 [%2]").
		arg(data::datautil::format_quantity(y_quantity()), y_unit_str());
}

void ScopeCurve::set_color(const QColor &custom_color)
{
	if (custom_color.isValid()) {
		has_custom_color_ = true;
		color_ = custom_color;
	}
	else {
		has_custom_color_ = false;
		color_ = ScopeCurve::default_color(signal_->name());
	}

	QPen pen = plot_curve_->pen();
	pen.setColor(color_);
	plot_curve_->setPen(pen);
}

QColor ScopeCurve::color() const
{
	return color_;
}

void ScopeCurve::set_style(const Qt::PenStyle style)
{
	QPen pen = plot_curve_->pen();
	pen.setStyle(style);
	plot_curve_->setPen(pen);
}

Qt::PenStyle ScopeCurve::style() const
{
	return plot_curve_->pen().style();
}

void ScopeCurve::set_symbol(const QwtSymbol::Style style)
{
	QwtSymbol *symbol = new QwtSymbol(style);
	symbol->setBrush(QBrush(color_));
	symbol->setPen(color_, 2);
	if (style == QwtSymbol::XCross)
		symbol->setSize(QSize(8, 8));
	else
		symbol->setSize(QSize(4, 4));
	plot_curve_->setSymbol(symbol);
}

QwtSymbol::Style ScopeCurve::symbol() const
{
	return plot_curve_->symbol()->style();
}

// TODO: name with _! Needed?
QRectF ScopeCurve::boundingRect() const
{
	if (curve_data_ == nullptr)
		return QRectF(0, 0, 0, 0); // TODO
	return curve_data_->boundingRect();
}

void ScopeCurve::attach(QwtPlot *plot)
{
	plot_curve_->attach(plot);

	const QwtScaleMap x_map = plot->canvasMap(x_axis_id());
	const QwtScaleMap y_map = plot->canvasMap(y_axis_id());
	curve_data_->update_scale_maps(x_map, y_map);

	connect(qobject_cast<PlotCanvas *>(plot->canvas()), &PlotCanvas::size_changed,
		this, &ScopeCurve::scale_maps_updated);
}

void ScopeCurve::detach()
{
	plot_curve_->detach();
}

void ScopeCurve::update()
{
	const QwtScaleMap x_map = plot_curve_->plot()->canvasMap(x_axis_id());
	const QwtScaleMap y_map = plot_curve_->plot()->canvasMap(y_axis_id());
	//curve_data_->update_scale_maps(x_map, y_map);

	const size_t num_points = curve_data_->size();
// 	qWarning() << "ScopeCurve::update(): painted_points = " << painted_points_;
// 	qWarning() << "ScopeCurve::update(): num_points = " << num_points;
// 	qWarning() << "ScopeCurve::update(): num_samples = " << curve_data_->size();
	if (num_points > painted_points_) {
		// TODO: move to somewhere else? -> We have a plot()->replot() and also a replot() here.... painted_points_ = 0....
		// TODO: What if we have multiple curves, that need a replot at different times?
		// TODO: If only a part of the curve is shown, do a replot not here  but just before the shown part (like pv)
		if (painted_points_ == 0)
			plot_curve_->plot()->replot();

		//qWarning() << QString("ScopeCurve::update(): num_points = %1, painted_points = %2").
		//	arg(num_points).arg(painted_points_);
		const bool clip = !plot_curve_->plot()->canvas()->testAttribute(Qt::WA_PaintOnScreen);
// 		qWarning() << "ScopeCurve::update(): clip = " << clip;
		if (clip) {
			// TODO: POI/ROI is now in CurveData. Is this needed any longer?
			/*
			 * NOTE:
			 * Depending on the platform setting a clip might be an
			 * important performance issue. F.e. for Qt Embedded this
			 * reduces the part of the backing store that has to be copied
			 * out - maybe to an unaccelerated frame buffer device.
			 */

			//const QwtScaleMap x_map = plot_curve_->plot()->canvasMap(x_axis_id());
			//const QwtScaleMap y_map = plot_curve_->plot()->canvasMap(y_axis_id());
			// TODO: Use bounding rect from CurveData?
			QRectF br = qwtBoundingRect(*plot_curve_->data(),
				(int)painted_points_ , (int)num_points - 1);

			plot_direct_painter_->setClipRegion(
				QwtScaleMap::transform(x_map, y_map, br).toRect());
		}

		qWarning() << "ScopeCurve::update(): Update " << QString::fromStdString(id());
		qWarning() << "ScopeCurve::update(): drawSeries(" << plot_curve_ << ", "
			<< (int)painted_points_ << ", " << (int)num_points - 1 << ")";

		qWarning() << "ScopeCurve::update(): drawSeries(" << plot_curve_ << "): first ts = "
			<< curve_data_->sample(0);
		qWarning() << "ScopeCurve::update(): drawSeries(" << plot_curve_ << "): start ts = "
			<< curve_data_->sample((int)painted_points_);
		qWarning() << "ScopeCurve::update(): drawSeries(" << plot_curve_ << "): last ts = "
			<< curve_data_->sample((int)num_points - 1);

		plot_direct_painter_->drawSeries(
			plot_curve_, (int)painted_points_, (int)num_points - 1);

		painted_points_ = num_points - 1;
		Q_EMIT new_points(); // TODO: rename to update_scale, move to boundary? not a good solution to update scale (changes)
	}

	//plot->replot();
}

void ScopeCurve::replot()
{
	qWarning() << "ScopeCurve::replot(): ==== Replot ====";
	// TODO
	//painted_points_ = 0;
	scale_maps_updated();
}

void ScopeCurve::reset()
{
	qWarning() << "ScopeCurve::reset(): ==== Reset ====";
	// TODO
	plot_direct_painter_->reset();
}

void ScopeCurve::new_segment(uint32_t segment_id)
{
	qWarning() << "ScopeCurve::new_segment(): ==== NEW Segment ====";

	curve_data_ = new ScopeCurveData(signal_->get_segment(segment_id));
	plot_curve_->setData(curve_data_);

	painted_points_ = 0;

	// TODO: move to somewhere else? -> We have a plot()->replot() and also a replot() here.... painted_points_ = 0....
	// TODO: What if we have multiple curves, that need a replot at different times?
	// TODO: If only a part of the curve is shown, do a replot not here  but just before the shown part (like pv)
	if (plot_curve_->plot() != nullptr)
		plot_curve_->plot()->replot();

	// TODO: BaseCurveData::reset_curve is not used?
	/*
	connect(curve_data_, &BaseCurveData::reset_curve,
		this, &ScopeCurve::on_reset);
	*/
}

void ScopeCurve::scale_maps_updated()
{
	curve_data_->update_scale_maps(
		plot_curve_->plot()->canvasMap(x_axis_id()),
		plot_curve_->plot()->canvasMap(y_axis_id()));
}

QwtPlotMarker *ScopeCurve::add_marker(const QString &name_postfix)
{
	QwtSymbol *symbol = new QwtSymbol(
		QwtSymbol::Diamond, QBrush(color_), QPen(color_), QSize(9, 9));
	QString name = QString("M%1").arg(name_postfix);

	QwtPlotMarker *marker = new QwtPlotMarker(name);
	marker->setSymbol(symbol);
	marker->setLineStyle(QwtPlotMarker::Cross);
	marker->setLinePen(Qt::white, 1.0, Qt::DashLine);
	marker->setXAxis(x_axis_id());
	marker->setYAxis(y_axis_id());
	// Markers will be painted ontop of curves but below the markers label box.
	marker->setZ(2);

	// Label
	QwtText marker_label = QwtText(name);
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

	return marker;
}

void ScopeCurve::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	settings.beginGroup(QString::fromStdString(id_));

	SettingsManager::save_signal(signal_, settings, origin_device);
	settings.setValue("x_axis_id", QVariant::fromValue(x_axis_id()));
	settings.setValue("y_axis_id", QVariant::fromValue(y_axis_id()));
	if (has_custom_name_)
		settings.setValue("custom_name", name_);
	if (has_custom_color_)
		settings.setValue("custom_color", color_);
	// Qt::PenSytle cannot be saved directly
	settings.setValue("style", QVariant(QPen(style())));
	settings.setValue("symbol", symbol());

	settings.endGroup();
}

ScopeCurve *ScopeCurve::init_from_settings(
	Session &session, QSettings &settings, const QString &group,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	if (!group.startsWith("scopecurve:"))
		return nullptr;

	settings.beginGroup(group);

	auto signal = SettingsManager::restore_signal(
		session, settings, origin_device);
	if (!signal)
		return nullptr;
	auto as_signal = dynamic_pointer_cast<sv::data::AnalogScopeSignal>(signal);
	if (!as_signal)
		return nullptr;

	ScopeCurve *curve = new ScopeCurve(as_signal,
		settings.value("x_axis_id").value<QwtPlot::Axis>(),
		settings.value("y_axis_id").value<QwtPlot::Axis>());
	if (settings.contains("custom_name"))
		curve->set_name(settings.value("custom_name").toString());
	if (settings.contains("custom_color"))
		curve->set_color(settings.value("custom_color").value<QColor>());
	if (settings.contains("style"))
		curve->set_style(settings.value("style").value<QPen>().style());
	if (settings.contains("symbol"))
		curve->set_symbol(settings.value("symbol").value<QwtSymbol::Style>());

	settings.endGroup();

	return curve;
}

QString ScopeCurve::get_unit_str(shared_ptr<sv::data::BaseSignal> signal)
{
	// TODO: use above
	return data::datautil::format_unit(
		signal->unit(), signal->quantity_flags());
}

QColor ScopeCurve::default_color(const string &channel_name)
{
	// Get the channel number
	std::regex rgx("(A|CH)([0-9]+).*");
	std::smatch matches;
	string channel_number = "";
	if (std::regex_search(channel_name, matches, rgx)  && matches.size() > 2)
		channel_number = matches.str(2);

	// First, try to get color from QSettings
	QColor color = QColor();
	QString key = QString("CH%1").arg(QString::fromStdString(channel_number));
	QSettings settings;
	if (SettingsManager::restore_settings() &&
			settings.childGroups().contains("DefaultScopeCurveColors")) {
		settings.beginGroup("DefaultScopeCurveColors");
		if (settings.childKeys().contains(key))
			color = settings.value(key).value<QColor>();
		settings.endGroup();
	}
	if (color.isValid())
		return color;

	// Predefined colors for eight channels, channel 0 is a special case for
	// the demo device
	if (channel_number == "0")
		return Qt::gray;
	if (channel_number == "1")
		return Qt::yellow;
	if (channel_number == "2")
		return Qt::cyan;
	if (channel_number == "3")
		return Qt::red;
	if (channel_number == "4")
		return Qt::green;
	if (channel_number == "5")
		return Qt::darkYellow;
	if (channel_number == "6")
		return Qt::darkBlue;
	if (channel_number == "7")
		return Qt::magenta;
	if (channel_number == "8")
		return Qt::darkGreen;

	// Random color for the rest
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
	return QColor::fromRgb(QRandomGenerator::global()->generate());
#else
	return QColor::fromRgb(qrand());
#endif
}

void ScopeCurve::save_settings_default_color(const string &channel_name,
	const QColor &color)
{
	// Get the channel number
	std::regex rgx("(A|CH)([0-9]+).*");
	std::smatch matches;
	string channel_number = "";
	if (std::regex_search(channel_name, matches, rgx)  && matches.size() > 2)
		channel_number = matches.str(2);
	QString key = QString("CH%1").arg(QString::fromStdString(channel_number));

	QSettings settings;
	settings.beginGroup("DefaultScopeCurveColors");
	settings.setValue(key, QVariant::fromValue(color));
	settings.endGroup();
}

void ScopeCurve::on_reset()
{
	/*
	painted_points_ = 0;
	Q_EMIT reset();
	*/
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

