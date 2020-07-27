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

#ifndef UI_WIDGETS_PLOT_CURVE_HPP
#define UI_WIDGETS_PLOT_CURVE_HPP

#include <string>
#include <vector>

#include <QColor>
#include <QObject>
#include <QSettings>
#include <QString>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

#include "src/data/datautil.hpp"

using std::string;
using std::vector;

namespace sv {

class Session;

namespace ui {
namespace widgets {
namespace plot {

class BaseCurveData;

class Curve : public QObject
{
	Q_OBJECT

public:
	Curve(BaseCurveData *curve_data, int x_axis_id, int y_axis_id,
		  QString custom_name = "", QColor custom_color = QColor());
	~Curve();

	static QColor default_color(sv::data::Quantity quantity,
		set<sv::data::QuantityFlag> quantity_flags);
	static void save_settings_default_color(sv::data::Quantity quantity,
		set<sv::data::QuantityFlag> quantity_flags, QColor &color);

	void save_settings(QSettings &settings) const;
	static Curve *init_from_settings(Session &session, QSettings &settings,
		const QString &group);

	BaseCurveData *curve_data() const;
	QwtPlotCurve *plot_curve() const;
	QwtPlotDirectPainter *plot_direct_painter() const;
	void set_name(const QString &custom_name);
	QString name() const;
	string id() const;
	int x_axis_id() const;
	int y_axis_id() const;
	void set_painted_points(size_t painted_points);
	size_t painted_points() const;
	void set_color(const QColor &custom_color);
	QColor color() const;
	void set_style(const Qt::PenStyle style);
	Qt::PenStyle style() const;
	void set_symbol(const QwtSymbol::Style style);
	QwtSymbol::Style symbol() const;
	QwtPlotMarker *add_marker(const QString &name_postfix);

private:
	BaseCurveData *curve_data_;
	QwtPlotCurve *plot_curve_;
	QwtPlotDirectPainter *plot_direct_painter_;
	bool has_custom_name_;
	QString name_;
	string id_;
	size_t painted_points_;
	bool has_custom_color_;
	QColor color_;

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_CURVE_HPP
