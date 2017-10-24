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

#include <QWidget>
#include <QDoubleSpinBox>
#include <qwt_knob.h>

#include "lcddisplay.hpp"

namespace sv {
namespace widgets {

class PowerPanel : public QWidget
{
    Q_OBJECT

public:
	PowerPanel(QWidget *parent);

private:
	widgets::LcdDisplay *voltageDisplay;
	widgets::LcdDisplay *currentDisplay;
	widgets::LcdDisplay *resistanceDisplay;
	widgets::LcdDisplay *powerDisplay;
	widgets::LcdDisplay *ampHourDisplay;
	widgets::LcdDisplay *wattHourDisplay;

	void setup_ui();
};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_POWERPANEL_HPP
