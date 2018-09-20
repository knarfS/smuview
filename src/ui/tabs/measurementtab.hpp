/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_TABS_MEASUREMENTTAB_HPP
#define UI_TABS_MEASUREMENTTAB_HPP

#include <memory>

#include <QMainWindow>

#include "src/ui/tabs/devicetab.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
class MeasurementDevice;
}

namespace ui {
namespace tabs {

class MeasurementTab : public DeviceTab
{
    Q_OBJECT

public:
	MeasurementTab(Session &session,
 		shared_ptr<sv::devices::MeasurementDevice> device, QMainWindow *parent);

private:
	void setup_ui();

	// TODO: remove, generic solution in hw_device
	shared_ptr<sv::devices::MeasurementDevice> measurement_device_;

};

} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_MEASUREMENTTAB_HPP
