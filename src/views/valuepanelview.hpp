/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef VIEWS_VALUEPANELVIEW_HPP
#define VIEWS_VALUEPANELVIEW_HPP

#include <memory>

#include <QAction>
#include <QString>
#include <QTimer>
#include <QToolBar>

#include "src/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace channels {
class BaseChannel;
}

namespace data {
class AnalogSignal;
}

namespace widgets {
class LcdDisplay;
}

namespace views {

class ValuePanelView : public BaseView
{
	Q_OBJECT

public:
	ValuePanelView(const Session& session,
		shared_ptr<channels::BaseChannel> channel,
		QWidget* parent = nullptr);

	ValuePanelView(const Session& session,
		shared_ptr<data::AnalogSignal> signal,
		QWidget* parent = nullptr);

	~ValuePanelView();

	QString title() const;

private:
	shared_ptr<channels::BaseChannel> channel_;
	shared_ptr<data::AnalogSignal> signal_;
	QString unit_;
	int digits_;
	int decimal_places_;

	QTimer *timer_;

	// Min/max/actual values are stored here, so they can be reseted
	double value_min_;
	double value_max_;

	QAction *const action_reset_display_;
	QToolBar *toolbar_;
	widgets::LcdDisplay *valueDisplay;
	widgets::LcdDisplay *valueMinDisplay;
	widgets::LcdDisplay *valueMaxDisplay;

	void setup_ui();
	void setup_toolbar();
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
} // namespace sv

#endif // VIEWS_VALUEPANELVIEW_HPP

