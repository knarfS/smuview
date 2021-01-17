/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_TABS_OSCILLOSCOPETAB_HPP
#define UI_TABS_OSCILLOSCOPETAB_HPP

#include <memory>

#include <QWidget>

#include "src/ui/tabs/devicetab.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace devices {
class HardwareDevice;
}

namespace ui {
namespace tabs {

class OscilloscopeTab : public DeviceTab
{
	Q_OBJECT

public:
	OscilloscopeTab(Session &session,
		shared_ptr<sv::devices::HardwareDevice> device,
		QWidget *parent = nullptr);

private:
	void setup_ui();

};

} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_OSCILLOSCOPETAB_HPP
