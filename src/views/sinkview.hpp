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

#ifndef VIEWS_SINKVIEW_HPP
#define VIEWS_SINKVIEW_HPP

#include <memory>

#include <QWidget>
#include <qwt_plot.h>

#include "src/devices/hardwaredevice.hpp"

using std::shared_ptr;

namespace sv {

namespace widgets {
class ControlButton;
class LcdDisplay;
class PowerPanel;
class ValueControl;
}

namespace views {

class SinkView : public QWidget
{
    Q_OBJECT

public:
	SinkView(shared_ptr<devices::HardwareDevice> device, QWidget *parent);

private:
	widgets::ControlButton *setEnableButton;
	widgets::ValueControl *setValueControl;
	widgets::PowerPanel *powerPanel;
	QwtPlot *plot;

	shared_ptr<devices::HardwareDevice> device_;

	void init_values();
	void setup_ui();

public Q_SLOTS:
	void on_value_changed(const double value);
	void on_enabled_changed(const bool enabled);
	//void on_data_received(shared_ptr<sv::data::AnalogSegment>);
};

} // namespace views
} // namespace sv

#endif // VIEWS_SINKVIEW_HPP
