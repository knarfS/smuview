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

#ifndef UI_WIDGETS_PLOT_SCOPEPLOT_HPP
#define UI_WIDGETS_PLOT_SCOPEPLOT_HPP

#include <map>
#include <vector>

#include <QVariant>

#include <qwt_interval.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_system_clock.h>
#include <qwt_text.h>

#include "src/data/datautil.hpp"

using std::map;
using std::pair;
using std::vector;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

class BaseCurveData;

class ScopePlot : public QwtPlot
{
	Q_OBJECT

public:
	ScopePlot(uint64_t samplerate, int num_hdiv, sv::data::rational_t timebase,
		QWidget *parent = nullptr);
	virtual ~ScopePlot();

	virtual void replot() override;
	virtual bool eventFilter(QObject * object, QEvent *event) override;

	/**
	* Add a new channel to the plot.
	* At the moment only two channels per plot are supported.
	*
	* @param name The new of the new channel
	*
	* @return true, if channel added.
	*/
	bool add_channel(QString name);
	bool show_curve(QString ch_name, plot::BaseCurveData *curve_data);
	void set_plot_interval(int plot_interval) { plot_interval_ = plot_interval; }
	//map<QwtPlotMarker *, plot::BaseCurveData *> markers() { return marker_map_; }

public Q_SLOTS:
	void start();
	void stop();
	void update_samplerate(const QVariant samplerate);
	void update_num_hdiv(const QVariant num_hdiv);
	void update_timebase(const QVariant timebase);

	/*
	void add_marker(plot::BaseCurveData *curve_data);
	void add_diff_marker(QwtPlotMarker *marker1, QwtPlotMarker *marker2);
	void remove_marker();
	void on_marker_selected(const QPointF mouse_pos);
	void on_marker_moved(const QPointF mouse_pos);
	*/
	void on_legend_clicked(const QVariant &item_info, int index);

protected:
	virtual void showEvent(QShowEvent *) override;
	virtual void resizeEvent(QResizeEvent *) override;
	virtual void timerEvent(QTimerEvent *) override;

private:
	int init_x_axis();
	int init_y_axis(QString ch_name);
	void update_curves();
	void update_intervals();
	bool update_x_interval();
	bool update_y_interval(int y_axis_id);
	//void update_markers_label();

	/** Map the channel name to the Y axis ID. */
	map<QString, int> y_axis_id_map_;
	/** Map the Y axis ID to the curve data. */
	map<int, plot::BaseCurveData *> curve_data_map_;
	/** Map the Y axis IDs to the plot curve. */
	map<int, QwtPlotCurve *> plot_curve_map_;
	/** Map the Y axis IDs to the paint mapper. */
	map<int, QwtPlotDirectPainter *> plot_direct_painter_map_;
	/** Map the curve data to the already painted points. */
	map<plot::BaseCurveData *, size_t> painted_points_map_;

	uint64_t samplerate_;
	int num_hdiv_;
	sv::data::rational_t timebase_;

	int plot_interval_;
	int timer_id_;

	/*
	vector<QwtPlotMarker *> markers_;
	vector<pair<QwtPlotMarker *, QwtPlotMarker *>> diff_markers_;
	map<QwtPlotMarker *, plot::BaseCurveData *> marker_map_;
	QwtPlotMarker *active_marker_;
	QwtPlotTextLabel *markers_label_;
	QwtPlotPicker *marker_select_picker_;
	QwtPlotPicker *marker_move_picker_;
	*/

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_SCOPEPLOT_HPP
