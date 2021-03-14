/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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
#include <string>
#include <vector>

#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

//#include "src/ui/views/baseplotview.hpp"
#include "src/ui/views/baseview.hpp"

using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {

class Session;

namespace channels {
class ScopeChannel;
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

class ScopePlotView : public BaseView
{
	Q_OBJECT

public:
	ScopePlotView(Session &session, QUuid uuid = QUuid(),
		QWidget *parent = nullptr);

	QString title() const override;

	void save_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device = nullptr) const override;
	void restore_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device = nullptr) override;

	/**
	 * Set the scope device to get timebase and trigger information.
	 */
	void set_scope_device(shared_ptr<sv::devices::OscilloscopeDevice> device);
	/**
	 * Add a new channel to the scope plot.
	 */
	string add_channel(shared_ptr<channels::ScopeChannel> channel);

private:
	shared_ptr<channels::ScopeChannel> channel_1_; // TODO
	shared_ptr<channels::ScopeChannel> channel_2_; // TODO
	//vector<widgets::plot::BaseCurveData *> curves_;
	widgets::plot::ScopePlot *plot_;

	void setup_ui();
	void setup_toolbar();

protected Q_SLOTS:
	//void on_action_add_curve_triggered();
	void update_add_marker_menu();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_SCOPEPLOTVIEW_HPP
