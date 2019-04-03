/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_VALUEPANELVIEW_HPP
#define UI_VIEWS_VALUEPANELVIEW_HPP

#include <memory>
#include <set>

#include <QAction>
#include <QString>
#include <QTimer>
#include <QToolBar>

#include "src/data/datautil.hpp"
#include "src/ui/views/baseview.hpp"

using std::set;
using std::shared_ptr;

namespace sv {

class Session;

namespace channels {
class BaseChannel;
}
namespace data {
class AnalogSignal;
}

namespace ui {

namespace widgets {
class ValueDisplay;
}

namespace views {

class ValuePanelView : public BaseView
{
	Q_OBJECT

public:
	ValuePanelView(Session& session,
		shared_ptr<channels::BaseChannel> channel,
		QWidget* parent = nullptr);

	ValuePanelView(Session& session,
		shared_ptr<sv::data::AnalogSignal> signal,
		QWidget* parent = nullptr);

	~ValuePanelView();

	QString title() const;

private:
	shared_ptr<channels::BaseChannel> channel_;
	shared_ptr<sv::data::AnalogSignal> signal_;
	QString unit_;
	QString unit_suffix_;
	set<sv::data::QuantityFlag> quantity_flags_;
	set<sv::data::QuantityFlag> quantity_flags_min_;
	set<sv::data::QuantityFlag> quantity_flags_max_;
	int digits_;
	int decimal_places_;

	QTimer *timer_;

	// Min/max/actual values are stored here, so they can be reseted
	double value_min_;
	double value_max_;

	QAction *const action_reset_display_;
	QToolBar *toolbar_;
	widgets::ValueDisplay *value_display_;
	widgets::ValueDisplay *value_min_display_;
	widgets::ValueDisplay *value_max_display_;

	void setup_ui();
	void setup_toolbar();
	void setup_unit();
	void connect_signals_displays();
	void disconnect_signals_displays();
	void reset_display();
	void init_timer();
	void stop_timer();

private Q_SLOTS:
	void on_update();
	void on_signal_changed();
	void on_action_reset_display_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_VALUEPANELVIEW_HPP
