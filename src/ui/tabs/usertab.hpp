/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_TABS_USERTAB_HPP
#define UI_TABS_USERTAB_HPP

#include <QObject>
#include <QWidget>

#include "src/ui/tabs/devicetab.hpp"

namespace sv {

class Session;

namespace devices {
class UserDevice;
}

namespace ui {
namespace tabs {

class UserTab : public DeviceTab
{
	Q_OBJECT

public:
	UserTab(Session &session,
 		shared_ptr<sv::devices::UserDevice> device, QWidget *parent = nullptr);

protected:
	void save_settings() const override;
	void restore_settings() override;

};

} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_USERTAB_HPP
