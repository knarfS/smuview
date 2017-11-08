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

#ifndef VIEWS_SOURCEVIEW_HPP
#define VIEWS_SOURCEVIEW_HPP

#include <memory>

#include <QWidget>

#include "src/devices/hardwaredevice.hpp"

using std::shared_ptr;

namespace sv {

namespace widgets {
class ControlButton;
class PowerPanel;
class ValueControl;
}

namespace views {

class SourceView : public QWidget
{
    Q_OBJECT

public:
	SourceView(shared_ptr<devices::HardwareDevice> device, QWidget *parent);

private:
	widgets::ControlButton *setEnableButton;
	widgets::ValueControl *setVoltageControl;
	widgets::ValueControl *setCurrentControl;
	widgets::PowerPanel *powerPanel;

	shared_ptr<devices::HardwareDevice> device_;

	void init_values();
	void setup_ui();

public Q_SLOTS:
	void on_voltage_changed(double value);
	void on_current_changed(double value);
};

} // namespace views
} // namespace sv

#endif // VIEWS_SOURCEVIEW_HPP
