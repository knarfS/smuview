/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <QColor>
#include <QDebug>
#include <QObject>
#if QT_VERSION >= 0x050A00
	#include <QRandomGenerator>
#endif
#include <QPen>
#include <QSettings>
#include <QUuid>
#include <QVariant>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

#include "curve.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/data/datautil.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"
#include "src/ui/widgets/plot/timecurvedata.hpp"
#include "src/ui/widgets/plot/xycurvedata.hpp"

Q_DECLARE_METATYPE(QwtSymbol::Style)

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

Curve::Curve(BaseCurveData *curve_data, int x_axis_id, int y_axis_id,
		QString custom_name, QColor custom_color) :
	curve_data_(curve_data),
	plot_direct_painter_(new QwtPlotDirectPainter()),
	painted_points_(0)
{
	QUuid uuid = QUuid::createUuid();
	id_ = curve_data->id_prefix() + ":" +
		uuid.toString(QUuid::WithoutBraces).toStdString();


	QPen pen;
	//pen.setColor(color_);
	pen.setWidthF(2.0);
	pen.setStyle(Qt::SolidLine);
	pen.setCosmetic(false);

	plot_curve_ = new QwtPlotCurve();
	plot_curve_->setYAxis(y_axis_id);
	plot_curve_->setXAxis(x_axis_id);
	plot_curve_->setStyle(QwtPlotCurve::Lines);
	plot_curve_->setPen(pen);
	// Set empty symbol, used in the PlotCurveConfigDialog.
	plot_curve_->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol));
	plot_curve_->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	plot_curve_->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
	plot_curve_->setData(curve_data_);
	//plot_curve_->setRawSamples(); // TODO: is this an option?
	// Curves have the lowest z order, everything else will be painted ontop.
	plot_curve_->setZ(1);

	set_name(custom_name);
	set_color(custom_color);
}

Curve::~Curve()
{
	delete plot_curve_;
	delete plot_direct_painter_;
}


BaseCurveData *Curve::curve_data() const
{
	return curve_data_;
}

QwtPlotCurve *Curve::plot_curve() const
{
	return plot_curve_;
}

QwtPlotDirectPainter *Curve::plot_direct_painter() const
{
	return plot_direct_painter_;
}

void Curve::set_name(const QString &custom_name)
{
	if (custom_name.size() > 0) {
		has_custom_name_ = true;
		name_ = custom_name;
	}
	else {
		has_custom_name_ = false;
		name_ = curve_data_->name();
	}
	plot_curve_->setTitle(name_);
}

QString Curve::name() const
{
	return name_;
}

string Curve::id() const
{
	return id_;
}

int Curve::x_axis_id() const
{
	return plot_curve_->xAxis();
}

int Curve::y_axis_id() const
{
	return plot_curve_->yAxis();
}

void Curve::set_painted_points(size_t painted_points)
{
	painted_points_ = painted_points;
}

size_t Curve::painted_points() const
{
	return painted_points_;
}

void Curve::set_color(const QColor &custom_color)
{
	if (custom_color.isValid()) {
		has_custom_color_ = true;
		color_ = custom_color;
	}
	else {
		has_custom_color_ = false;
		color_ = Curve::default_color(
			curve_data_->y_quantity(), curve_data_->y_quantity_flags());
	}

	QPen pen = plot_curve_->pen();
	pen.setColor(color_);
	plot_curve_->setPen(pen);
}

QColor Curve::color() const
{
	return color_;
}

void Curve::set_style(const Qt::PenStyle style)
{
	QPen pen = plot_curve_->pen();
	pen.setStyle(style);
	plot_curve_->setPen(pen);
}

Qt::PenStyle Curve::style() const
{
	return plot_curve_->pen().style();
}

void Curve::set_symbol(const QwtSymbol::Style style)
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

QwtSymbol::Style Curve::symbol() const
{
	return plot_curve_->symbol()->style();
}

QwtPlotMarker *Curve::add_marker(const QString &name_postfix)
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

void Curve::save_settings(QSettings &settings) const
{
	settings.beginGroup(QString::fromStdString(id_));
	curve_data_->save_settings(settings);
	settings.setValue("x_axis_id", x_axis_id());
	settings.setValue("y_axis_id", y_axis_id());
	if (has_custom_name_)
		settings.setValue("custom_name", name_);
	if (has_custom_color_)
		settings.setValue("custom_color", color_);
	// Qt::PenSytle cannot be saved directly
	settings.setValue("style", QVariant(QPen(style())));
	settings.setValue("symbol", symbol());

	settings.endGroup();
}

Curve *Curve::init_from_settings(Session &session, QSettings &settings,
	const QString &group)
{
	if (!group.startsWith("timecurve:") && !group.startsWith("xycurve:"))
		return nullptr;

	settings.beginGroup(group);

	BaseCurveData *curve_data = nullptr;
	if (group.startsWith("timecurve:"))
		curve_data = TimeCurveData::init_from_settings(session, settings);
	else if (group.startsWith("xycurve:"))
		curve_data = XYCurveData::init_from_settings(session, settings);
	if (!curve_data) {
		settings.endGroup();
		return nullptr;
	}

	int x_axis_id = settings.value("x_axis_id").toInt();
	int y_axis_id = settings.value("y_axis_id").toInt();
	QString custom_name = "";
	if (settings.contains("custom_name"))
		custom_name = settings.value("custom_name").toString();
	QColor custom_color = QColor();
	if (settings.contains("custom_color"))
		custom_color = settings.value("custom_color").value<QColor>();

	Curve *curve = new Curve(curve_data, x_axis_id, y_axis_id,
		custom_name, custom_color);

	if (settings.contains("style"))
		curve->set_style(settings.value("style").value<QPen>().style());
	if (settings.contains("symbol"))
		curve->set_symbol(settings.value("symbol").value<QwtSymbol::Style>());

	settings.endGroup();

	return curve;
}

QColor Curve::default_color(sv::data::Quantity quantity,
	set<sv::data::QuantityFlag> quantity_flags)
{
	// First, try to get color from QSettings
	QColor color = QColor();
	QSettings settings;
	if (SettingsManager::restore_settings() &&
			settings.childGroups().contains("DefaultCurveColors")) {
		settings.beginGroup("DefaultCurveColors");
		QString key = QString("%1_%2").
			arg(data::datautil::get_sr_quantity_id(quantity)).
			arg(data::datautil::get_sr_quantity_flags_id(quantity_flags));
		if (settings.childKeys().contains(key))
			color = settings.value(key).value<QColor>();
		settings.endGroup();
	}
	if (color.isValid())
		return color;

	// Predefined colors
	if (quantity == sv::data::Quantity::Voltage &&
			quantity_flags.count(sv::data::QuantityFlag::DC) > 0)
		return Qt::red;
	if (quantity == sv::data::Quantity::Voltage &&
			quantity_flags.count(sv::data::QuantityFlag::AC) > 0)
		return Qt::darkRed;
	if (quantity == sv::data::Quantity::Voltage)
		// Fallback for Voltage without quantity flag
		return Qt::red;
	if (quantity == sv::data::Quantity::Current &&
			quantity_flags.count(sv::data::QuantityFlag::DC) > 0)
		return Qt::green;
	if (quantity == sv::data::Quantity::Current &&
			quantity_flags.count(sv::data::QuantityFlag::AC) > 0)
		return Qt::darkGreen;
	if (quantity == sv::data::Quantity::Current)
		// Fallback for current without quantity flag
		return Qt::green;
	if (quantity == sv::data::Quantity::Resistance)
		return Qt::cyan;
	if (quantity == sv::data::Quantity::Power)
		return Qt::yellow;
	if (quantity == sv::data::Quantity::Energy)
		return Qt::darkYellow;
	if (quantity == sv::data::Quantity::Temperature)
		return Qt::darkCyan;
	if (quantity == sv::data::Quantity::Capacitance)
		return Qt::gray;
	if (quantity == sv::data::Quantity::Frequency)
		return Qt::magenta;
	if (quantity == sv::data::Quantity::Time)
		return Qt::darkMagenta;
	if (quantity == sv::data::Quantity::PowerFactor)
		return Qt::lightGray;
	if (quantity == sv::data::Quantity::ElectricCharge)
		return Qt::darkGray;

	// Random color for the rest
#if QT_VERSION >= 0x050A00
	return QColor::fromRgb(QRandomGenerator::global()->generate());
#else
	return QColor::fromRgb(qrand());
#endif
}

void Curve::save_settings_default_color(sv::data::Quantity quantity,
	set<sv::data::QuantityFlag> quantity_flags, QColor &color)
{
	QSettings settings;
	settings.beginGroup("DefaultCurveColors");
	QString key = QString("%1_%2").
		arg(data::datautil::get_sr_quantity_id(quantity)).
		arg(data::datautil::get_sr_quantity_flags_id(quantity_flags));
	settings.setValue(key, QVariant::fromValue(color));
	settings.endGroup();
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
