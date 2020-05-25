/*
 * This file is part of the SmuView project.
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

#ifndef UI_VIEWS_PLOTVIEW_HPP
#define UI_VIEWS_PLOTVIEW_HPP

#include <memory>
#include <vector>

#include <QAction>
#include <QMenu>
#include <QSettings>
#include <QToolBar>
#include <QToolButton>
#include <QUuid>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {

class Session;

namespace channels {
class BaseChannel;
}
namespace data {
class AnalogTimeSignal;
}

namespace ui {

namespace widgets {
namespace plot {
class BaseCurveData;
class Plot;
}
}

namespace views {

enum class PlotType {
	TimePlot,
	XYPlot,
};

class PlotView : public BaseView
{
	Q_OBJECT

public:
	PlotView(Session &session,
		shared_ptr<channels::BaseChannel> channel,
		QUuid uuid = QUuid(),
		QWidget *parent = nullptr);
	PlotView(Session &session,
		shared_ptr<sv::data::AnalogTimeSignal> signal,
		QUuid uuid = QUuid(),
		QWidget *parent = nullptr);
	PlotView(Session &session,
		shared_ptr<sv::data::AnalogTimeSignal> x_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_signal,
		QUuid uuid = QUuid(),
		QWidget *parent = nullptr);

	QString title() const override;

	// TODO: scope
	void save_settings(QSettings &settings) const override;
	void restore_settings(QSettings &settings) override;

	/**
	 * Add a new signal to the time plot.
	 */
	void add_time_curve(shared_ptr<sv::data::AnalogTimeSignal> signal);
	/**
	 * Add a new signal to the xy plot. The new signal will be time correlated
	 * with the already set x signal.
	 */
	void add_xy_curve(shared_ptr<sv::data::AnalogTimeSignal> y_signal);
	/**
	 * Add a new x/y curve to the xy plot.
	 */
	void add_xy_curve(shared_ptr<sv::data::AnalogTimeSignal> x_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_signal);

private:
	void setup_ui();
	void setup_toolbar();
	void update_add_marker_menu();
	void connect_signals();
	void init_values();

	shared_ptr<channels::BaseChannel> initial_channel_;
	vector<widgets::plot::BaseCurveData *> curves_;

	QMenu *add_marker_menu_;
	QToolButton *add_marker_button_;
	QAction *const action_add_marker_;
	QAction *const action_add_diff_marker_;
	QAction *const action_zoom_best_fit_;
	QAction *const action_add_signal_;
	QAction *const action_save_;
	QAction *const action_config_plot_;
	QToolBar *toolbar_;
	widgets::plot::Plot *plot_;
	PlotType plot_type_;

private Q_SLOTS:
	void on_signal_changed();
	void on_action_add_marker_triggered();
	void on_action_add_diff_marker_triggered();
	void on_action_zoom_best_fit_triggered();
	void on_action_add_signal_triggered();
	void on_action_save_triggered();
	void on_action_config_plot_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_PLOTVIEW_HPP
