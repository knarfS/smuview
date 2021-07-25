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

#ifndef UI_WIDGETS_PLOT_SCOPECURVE_HPP
#define UI_WIDGETS_PLOT_SCOPECURVE_HPP

#include <memory>
#include <string>
#include <vector>

#include <QColor>
#include <QObject>
#include <QRectF>
#include <QSettings>
#include <QString>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_map.h>
#include <qwt_symbol.h>

#include "src/data/datautil.hpp"

using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {

class Session;

namespace data {
class AnalogScopeSignal;
class BaseSignal;
}
namespace devices {
class BaseDevice;
}

namespace ui {
namespace widgets {
namespace plot {

//class BaseCurveData;
class ScopeCurveData;
//class TestArrayPlotItem;

enum class CurveType {
	ScopeCurve,
	TimeCurve,
	XYCurve
};

class ScopeCurve : public QObject // TODO: Inherit from Curve
{
	Q_OBJECT

public:
	ScopeCurve(shared_ptr<sv::data::AnalogScopeSignal> signal,
		int x_axis_id = -1, int y_axis_id = -1);
	~ScopeCurve();

	static QString get_unit_str(shared_ptr<sv::data::BaseSignal> signal);
	static QColor default_color(const string &channel_name);
	static void save_settings_default_color(const string &channel_name,
		const QColor &color);

	CurveType type() const;
	string id() const;
	void set_name(const QString &custom_name);
	QString name() const; // TODO: override
	string id_prefix() const; // TODO: override
	void set_relative_time(bool is_relative_time);
	bool is_relative_time() const;

	void set_x_axis_id(int x_axis_id);
	QwtPlot::Axis x_axis_id() const;
	void set_y_axis_id(int y_axis_id);
	QwtPlot::Axis y_axis_id() const;

	sv::data::Quantity x_quantity() const; // TODO: override
	set<sv::data::QuantityFlag> x_quantity_flags() const; // TODO: override
	sv::data::Unit x_unit() const; // TODO: override
	QString x_unit_str() const; // TODO: override
	QString x_title() const; // TODO: override
	sv::data::Quantity y_quantity() const; // TODO: override
	set<sv::data::QuantityFlag> y_quantity_flags() const; // TODO: override
	sv::data::Unit y_unit() const; // TODO: override
	QString y_unit_str() const; // TODO: override
	QString y_title() const; // TODO: override

	void set_color(const QColor &custom_color);
	QColor color() const;
	void set_style(const Qt::PenStyle style);
	Qt::PenStyle style() const;
	void set_symbol(const QwtSymbol::Style style);
	QwtSymbol::Style symbol() const;

	QRectF boundingRect() const; // TODO: name with _! Needed?

	void attach(QwtPlot *plot);
	void detach();
	void update();
	void replot();
	void reset();
	QwtPlotMarker *add_marker(const QString &name_postfix);

	void save_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device) const;
	static ScopeCurve *init_from_settings(
		Session &session, QSettings &settings, const QString &group,
		shared_ptr<sv::devices::BaseDevice> origin_device);

private:
	shared_ptr<sv::data::AnalogScopeSignal> signal_;
	//BaseCurveData *curve_data_;
	ScopeCurveData *curve_data_; // TODO: BaseCurveData, Rename: current_curve_data_
	QwtPlotCurve *plot_curve_;
	QwtPlotDirectPainter *plot_direct_painter_;
	const CurveType type_;
	bool relative_time_;
	bool has_custom_name_;
	QString name_;
	string id_;
	size_t painted_points_;
	bool has_custom_color_;
	QColor color_;



private Q_SLOTS:
	void on_reset();
	void new_segment(uint32_t segment_id); // TODO: rename?
	void update_x_scale();
	void update_y_scale();

Q_SIGNALS:
	void new_points();
	//void reset();

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_SCOPECURVE_HPP
