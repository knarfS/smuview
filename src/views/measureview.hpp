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

#ifndef VIEWS_MEASUREVIEW_HPP
#define VIEWS_MEASUREVIEW_HPP

#include <memory>

#include <QWidget>

#include "src/devices/hardwaredevice.hpp"

using std::shared_ptr;

namespace sv {

namespace widgets {
class SingleValuePanel;
class Plot;
}

namespace views {

class MeasureView : public QWidget
{
    Q_OBJECT

public:
	MeasureView(shared_ptr<devices::HardwareDevice> device, QWidget *parent);

private:
	shared_ptr<devices::HardwareDevice> device_;
	uint digits_;
	QString unit_;

	widgets::SingleValuePanel *singleValuePanel;
	widgets::Plot *plot;

	void init_values();
	void setup_ui();

};

} // namespace views
} // namespace sv

#endif // VIEWS_MEASUREVIEW_HPP
