/*
 * This file is part of the SmuView project.
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

#ifndef UI_VIEWS_SCOPEPLOTVIEW_HPP
#define UI_VIEWS_SCOPEPLOTVIEW_HPP

#include <memory>
#include <vector>

#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

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
namespace devices {
class OscilloscopeDevice;
}

namespace ui {

namespace widgets {
namespace plot {
class BaseCurveData;
class ScopePlot;
}
}

namespace views {

/* TODO
enum class PlotType {
	TimePlot,
	XYPlot,
};
*/

class ScopePlotView : public BaseView
{
	Q_OBJECT

public:
	ScopePlotView(Session &session,
		shared_ptr<sv::devices::OscilloscopeDevice> device,
		shared_ptr<channels::BaseChannel> channel,
		QWidget *parent = nullptr);
	ScopePlotView(Session &session,
		shared_ptr<sv::devices::OscilloscopeDevice> device,
		shared_ptr<channels::BaseChannel> x_channel,
		shared_ptr<channels::BaseChannel> y_channel,
		QWidget *parent = nullptr);

	QString title() const;
	void add_channel(shared_ptr<channels::BaseChannel> channel);

private:
	void setup_ui();
	void setup_toolbar();
	void update_add_marker_menu();
	void connect_signals();
	void init_values();

	shared_ptr<sv::devices::OscilloscopeDevice> device_;
	shared_ptr<channels::BaseChannel> channel_1_;
	shared_ptr<channels::BaseChannel> channel_2_;
	vector<widgets::plot::BaseCurveData *> curves_;

	QMenu *add_marker_menu_;
	QToolButton *add_marker_button_;
	QAction *const action_add_marker_;
	QAction *const action_add_diff_marker_;
	QAction *const action_zoom_best_fit_;
	QAction *const action_add_signal_;
	QAction *const action_config_plot_;
	QToolBar *toolbar_;
	widgets::plot::ScopePlot *plot_;

private Q_SLOTS:
	void on_signal_added();
	void on_action_add_marker_triggered();
	void on_action_add_diff_marker_triggered();
	void on_action_zoom_best_fit_triggered();
	void on_action_add_signal_triggered();
	void on_action_config_plot_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_SCOPEPLOTVIEW_HPP
