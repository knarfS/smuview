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

#ifndef UI_WIDGETS_PLOT_HPP
#define UI_WIDGETS_PLOT_HPP

#include <map>
#include <vector>

#include <QVariant>

#include <qwt_interval.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_system_clock.h>

using std::map;
using std::vector;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

class BaseCurve;

enum class PlotUpdateMode {
	Additive = 0,
	Rolling,
	Oscilloscope
};

// TODO: Use tr(), QCoreApplication::translate(), QT_TR_NOOP() or
//       QT_TRANSLATE_NOOP() for translation.
//       See: http://doc.qt.io/qt-5/i18n-source-translation.html
typedef map<PlotUpdateMode, QString> plot_update_mode_name_map_t;
static plot_update_mode_name_map_t plot_update_mode_name_map = {
	{ sv::ui::widgets::plot::PlotUpdateMode::Additive, QString("Additive") },
	{ sv::ui::widgets::plot::PlotUpdateMode::Rolling, QString("Rolling") },
	{ sv::ui::widgets::plot::PlotUpdateMode::Oscilloscope, QString("Oscilloscope") },
};

class Plot : public QwtPlot
{
	Q_OBJECT

public:
	Plot(QWidget *parent = nullptr);
	virtual ~Plot();

	virtual void replot();
	virtual bool eventFilter(QObject * object, QEvent *event);
	void add_curve(plot::BaseCurve *curve_data);
	void set_plot_interval(int plot_interval) { plot_interval_ = plot_interval; }
	void set_update_mode(PlotUpdateMode update_mode) { update_mode_ = update_mode; }
	PlotUpdateMode update_mode() const { return update_mode_; };
	void set_time_span(double time_span) { time_span_ = time_span; }
	double time_span() { return time_span_; }
	void set_add_time(double add_time) { add_time_ = add_time; }
	double add_time() { return add_time_; }

public Q_SLOTS:
	void start();
	void stop();
	int init_x_axis(widgets::plot::BaseCurve *curve);
	int init_y_axis(widgets::plot::BaseCurve *curve);
	void set_x_interval(double x_start, double x_end);
	void set_y_interval(int y_axis_id, double y_start, double y_end);
	void set_x_axis_fixed(const bool fixed);
	void set_y_axis_fixed(const bool fixed);
	void add_marker();
	void on_marker_selected(const QPointF pos);
	void on_marker_moved(const QPoint pos);
	void on_legend_clicked(const QVariant &item_info, int index);

protected:
	virtual void showEvent(QShowEvent *);
	virtual void resizeEvent(QResizeEvent *);
	virtual void timerEvent(QTimerEvent *);

private:
	void update_curves();
	void update_intervals();
	bool update_x_interval(plot::BaseCurve *curve);
	bool update_y_interval(plot::BaseCurve *curve);

	vector<plot::BaseCurve *> curves_;
	map<plot::BaseCurve *, QwtPlotCurve *> plot_curve_map_;
	map<plot::BaseCurve *, QwtPlotDirectPainter *> plot_direct_painter_map_;
	map<plot::BaseCurve *, QwtInterval *> curve_y_interval_map_;
	map<plot::BaseCurve *, int> curve_y_axis_id_map_;
	map<plot::BaseCurve *, size_t> painted_points_map_;

	map<int, QwtInterval *> y_axis_interval_map_;

	QwtInterval x_interval_;
	bool x_axis_fixed_;
	bool y_axis_fixed_;
	int plot_interval_;
	int timer_id_;
	PlotUpdateMode update_mode_;
	double time_span_;
	double add_time_;

	vector<QwtPlotMarker *> markers_;
	QwtPlotMarker *active_marker_;
	QwtPlotPicker *picker_;

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_HPP
