/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef WIDGETS_POWERPANEL_HPP
#define WIDGETS_POWERPANEL_HPP

#include <memory>

#include <QDoubleSpinBox>
#include <QTimer>
#include <QWidget>
#include <qwt_knob.h>

#include "lcddisplay.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
class HardwareDevice;
}

namespace widgets {

class PowerPanel : public QWidget
{
    Q_OBJECT

public:
	PowerPanel(shared_ptr<devices::HardwareDevice> device, QWidget *parent);
	~PowerPanel();

private:
	shared_ptr<devices::HardwareDevice> device_;

	QTimer *timer_;
	qint64 start_time_;
	qint64 last_time_;
	double actual_amp_hours_;
	double actual_watt_hours_;

	widgets::LcdDisplay *voltageDisplay;
	widgets::LcdDisplay *currentDisplay;
	widgets::LcdDisplay *resistanceDisplay;
	widgets::LcdDisplay *powerDisplay;
	widgets::LcdDisplay *ampHourDisplay;
	widgets::LcdDisplay *wattHourDisplay;

	void setup_ui();
	void init_timer();
	void stop_timer();
	void reset();

public Q_SLOTS:
	void on_update();
};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_POWERPANEL_HPP
