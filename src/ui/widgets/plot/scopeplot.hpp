/*
 * This file is part of the SmuView project.
 * This file is based on the QWT Oscilloscope Example.
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

#ifndef UI_WIDGETS_PLOT_SCOPEPLOT_HPP
#define UI_WIDGETS_PLOT_SCOPEPLOT_HPP

#include <map>
#include <memory>
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

#include "src/data/datautil.hpp"
#include "src/ui/widgets/plot/plot.hpp"

using std::map;
using std::pair;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {

class Session;

namespace data {
class AnalogScopeSignal;
}
namespace devices {
class BaseDevice;
}

namespace ui {
namespace widgets {
namespace plot {

class PlotMagnifier;
class ScopeCurve;

class ScopePlot : public QwtPlot
{
	Q_OBJECT

public:
	explicit ScopePlot(Session &session, QWidget *parent = nullptr);
	virtual ~ScopePlot();

	virtual void replot() override;
	virtual bool eventFilter(QObject * object, QEvent *event) override;
	/**
	 * Return the id of the new curve. Empty string when curve couldn't be
	 * added.
	 */
	//string add_curve(ScopeCurveData *curve_data);
	bool add_curve(ScopeCurve *curve);
	void remove_curve(ScopeCurve *curve);
	void remove_all_curves();
	/** Return a map of all curves. */
	map<string, ScopeCurve *> curve_map() const { return curve_map_; }
	int get_free_x_axis(const QString &unit_str) const;
	int get_free_y_axis(const QString &unit_str) const;
	bool is_axis_locked(int axis_id, AxisBoundary axis_boundary) { return axis_lock_map_[axis_id][axis_boundary]; }
	void set_axis_locked(int axis_id, AxisBoundary axis_boundary, bool locked);
	void set_all_axis_locked(bool locked);
	void set_update_mode(PlotUpdateMode update_mode) { update_mode_ = update_mode; }
	PlotUpdateMode update_mode() const { return update_mode_; };
	void set_time_span(double time_span);
	double time_span() const { return time_span_; }
	void set_add_time(double add_time) { add_time_ = add_time; }
	double add_time() const { return add_time_; }
	map<QwtPlotMarker *, ScopeCurve *> marker_curve_map() const { return marker_curve_map_; }
	void set_markers_label_alignment(int alignment);
	int markers_label_alignment() const { return markers_label_alignment_; }

	void save_settings(QSettings &settings, bool save_curves,
		shared_ptr<sv::devices::BaseDevice> origin_device) const;
	void restore_settings(QSettings &settings, bool restore_curves,
		shared_ptr<sv::devices::BaseDevice> origin_device);

public Q_SLOTS:
	void add_axis_icons(const int axis_id);
	void lock_all_axis();
	void on_axis_lock_clicked();
	void add_marker(sv::ui::widgets::plot::ScopeCurve *curve);
	void add_diff_marker(QwtPlotMarker *marker1, QwtPlotMarker *marker2);
	void remove_marker(QwtPlotMarker *marker);
	void on_marker_selected(const QPointF mouse_pos);
	void on_marker_moved(const QPointF mouse_pos);
	void on_legend_clicked(const QVariant &item_info, int index);
	void update_samplerate(const QVariant samplerate);
	void update_num_hdiv(const QVariant num_hdiv);
	void update_timebase(const QVariant timebase);
	void update_trigger_source(const QVariant trigger_source);
	void update_trigger_level(const QVariant trigger_level);
	void update_horiz_trigger_pos(const QVariant horiz_trigger_pos);
	void update_curves();

private Q_SLOTS:
	void update_x_intervals();

protected:
	virtual void showEvent(QShowEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;

private:
	int init_x_axis(const ScopeCurve *curve, int x_axis_id = -1);
	int init_y_axis(const ScopeCurve *curve, int y_axis_id = -1);
	void init_axis(int axis_id, double min, double max, const QString &title,
			bool auto_scale);
	void update_intervals();
	bool update_x_interval(ScopeCurve *curve);
	bool update_y_interval(const ScopeCurve *curve);
	void update_trigger_marker();
	void update_markers_label();
	ScopeCurve *get_curve_from_plot_curve(const QwtPlotCurve *plot_curve) const;

	Session &session_;
	map<string, ScopeCurve *> curve_map_;
	map<int, map<AxisBoundary, bool>> axis_lock_map_; // map<axis_id, map<AxisBoundary, locked>>
	uint64_t samplerate_;
	int num_hdiv_;
	sv::data::rational_t timebase_;
	QString trigger_source_;
	double trigger_level_;
	double horiz_trigger_pos_;
	PlotUpdateMode update_mode_;
	double time_span_;
	double add_time_;

	QwtPlotPanner *plot_panner_;
	PlotMagnifier *plot_magnifier_;

	QwtPlotMarker *trigger_level_marker_;
	QwtPlotMarker *trigger_hpos_marker_;

	map<QwtPlotMarker *, ScopeCurve *> marker_curve_map_;
	vector<pair<QwtPlotMarker *, QwtPlotMarker *>> diff_markers_;
	QwtPlotMarker *active_marker_;
	QwtPlotTextLabel *markers_label_;
	int markers_label_alignment_;
	QwtPlotPicker *marker_select_picker_;
	QwtPlotPicker *marker_move_picker_;

Q_SIGNALS:
	void axis_lock_changed(int axis_id,
		sv::ui::widgets::plot::AxisBoundary axis_boundary, bool locked);
	void curve_added();
	void curve_removed();
	void marker_added();
	void marker_removed();

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_SCOPEPLOT_HPP
