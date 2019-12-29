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

#ifndef UI_VIEWS_POWERPANELVIEW_HPP
#define UI_VIEWS_POWERPANELVIEW_HPP

#include <memory>

#include <QAction>
#include <QTimer>
#include <QToolBar>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace data {
class AnalogTimeSignal;
}

namespace ui {

namespace widgets {
class ValueDisplay;
}

namespace views {

class PowerPanelView : public BaseView
{
	Q_OBJECT

public:
	PowerPanelView(Session& session,
		shared_ptr<sv::data::AnalogTimeSignal> voltage_signal,
		shared_ptr<sv::data::AnalogTimeSignal> current_signal,
		QWidget* parent = nullptr);
	~PowerPanelView();

	QString title() const override;

private:
	shared_ptr<sv::data::AnalogTimeSignal> voltage_signal_;
	shared_ptr<sv::data::AnalogTimeSignal> current_signal_;

	QTimer *timer_;
	qint64 start_time_;
	qint64 last_time_;

	// Min/max/actual values are stored here, so they can be reseted
	double voltage_min_;
	double voltage_max_;
	double current_min_;
	double current_max_;
	double resistance_min_;
	double resistance_max_;
	double power_min_;
	double power_max_;
	double actual_amp_hours_;
	double actual_watt_hours_;

	QAction *const action_reset_displays_;
	QToolBar *toolbar_;
	widgets::ValueDisplay *voltage_display_;
	widgets::ValueDisplay *voltage_min_display_;
	widgets::ValueDisplay *voltage_max_display_;
	widgets::ValueDisplay *current_display_;
	widgets::ValueDisplay *current_min_display_;
	widgets::ValueDisplay *current_max_display_;
	widgets::ValueDisplay *resistance_display_;
	widgets::ValueDisplay *resistance_min_display_;
	widgets::ValueDisplay *resistance_max_display_;
	widgets::ValueDisplay *power_display_;
	widgets::ValueDisplay *power_min_display_;
	widgets::ValueDisplay *power_max_display_;
	widgets::ValueDisplay *amp_hour_display_;
	widgets::ValueDisplay *watt_hour_display_;

	void setup_ui();
	void setup_toolbar();
	void connect_signals();
	void reset_displays();
	void init_timer();
	void stop_timer();

private Q_SLOTS:
	void on_update();
	void on_action_reset_displays_triggered();
	void on_digits_changed();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_POWERPANELVIEW_HPP
