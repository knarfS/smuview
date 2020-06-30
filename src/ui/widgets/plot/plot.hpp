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

#ifndef UI_WIDGETS_PLOT_PLOT_HPP
#define UI_WIDGETS_PLOT_PLOT_HPP

#include <map>
#include <string>
#include <vector>

#include <QSettings>
#include <QString>
#include <QVariant>

#include <qwt_interval.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_system_clock.h>
#include <qwt_text.h>

using std::string;
using std::map;
using std::pair;
using std::vector;

namespace sv {

class Session;

namespace ui {
namespace widgets {
namespace plot {

class BaseCurveData;
class Curve;
class PlotMagnifier;

enum class AxisBoundary {
	LowerBoundary,
	UpperBoundary
};

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
	Plot(Session &session, QWidget *parent = nullptr);
	virtual ~Plot();

	virtual void replot() override;
	virtual bool eventFilter(QObject * object, QEvent *event) override;
	/**
	 * Retruns the id of the new curve. Empty string when curve couldn't be
	 * added.
	 */
	string add_curve(BaseCurveData *curve_data);
	bool add_curve(Curve *curve);
	vector<Curve *> curves() const { return curves_; }
	bool is_axis_locked(int axis_id, AxisBoundary axis_boundary) { return axis_lock_map_[axis_id][axis_boundary]; }
	void set_axis_locked(int axis_id, AxisBoundary axis_boundary, bool locked);
	void set_all_axis_locked(bool locked);
	void set_plot_interval(int plot_interval) { plot_interval_ = plot_interval; }
	void set_update_mode(PlotUpdateMode update_mode) { update_mode_ = update_mode; }
	PlotUpdateMode update_mode() const { return update_mode_; };
	void set_time_span(double time_span);
	double time_span() const { return time_span_; }
	void set_add_time(double add_time) { add_time_ = add_time; }
	double add_time() const { return add_time_; }
	map<QwtPlotMarker *, Curve *> marker_curve_map() const { return marker_curve_map_; }
	void set_markers_label_alignment(int alignment);
	int markers_label_alignment() const { return markers_label_alignment_; }

	void save_settings(QSettings &settings, bool save_curves) const;
	void restore_settings(QSettings &settings, bool restore_curves);

public Q_SLOTS:
	void start();
	void stop();
	void add_axis_icons(const int axis_id);
	void lock_all_axis();
	void on_axis_lock_clicked();
	void add_marker(Curve *curve);
	void add_diff_marker(QwtPlotMarker *marker1, QwtPlotMarker *marker2);
	void remove_marker();
	void on_marker_selected(const QPointF mouse_pos);
	void on_marker_moved(const QPointF mouse_pos);
	void on_legend_clicked(const QVariant &item_info, int index);

Q_SIGNALS:
	void axis_lock_changed(int axis_id, AxisBoundary axis_boundary, bool locked);

protected:
	virtual void showEvent(QShowEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void timerEvent(QTimerEvent *event) override;

private:
	int init_x_axis(BaseCurveData *curve_data, int x_axis_id = -1);
	int init_y_axis(BaseCurveData *curve_data, int y_axis_id = -1);
	void init_axis(int axis_id, double min, double max, const QString &title,
		bool auto_scale);
	void update_curves();
	void update_intervals();
	bool update_x_interval(Curve *curve);
	bool update_y_interval(const Curve *curve);
	void update_markers_label();
	Curve *get_curve_from_plot_curve(const QwtPlotCurve *plot_curve) const;

	Session &session_;
	vector<Curve *> curves_; // TODO: shared_ptr/unique_ptr?
	map<int, map<AxisBoundary, bool>> axis_lock_map_; // map<axis_id, map<AxisBoundary, locked>>
	int plot_interval_;
	int timer_id_;
	PlotUpdateMode update_mode_;
	double time_span_;
	double add_time_;

	QwtPlotPanner *plot_panner_;
	PlotMagnifier *plot_magnifier_;

	map<QwtPlotMarker *, Curve *> marker_curve_map_;
	vector<pair<QwtPlotMarker *, QwtPlotMarker *>> diff_markers_;
	QwtPlotMarker *active_marker_;
	QwtPlotTextLabel *markers_label_;
	int markers_label_alignment_;
	QwtPlotPicker *marker_select_picker_;
	QwtPlotPicker *marker_move_picker_;

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_PLOT_HPP
