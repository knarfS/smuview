/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_TABS_TABHELPER_HPP
#define UI_TABS_TABHELPER_HPP

#include <memory>

#include <QWidget>

using std::shared_ptr;

namespace sv {

class Session;

namespace devices {
class BaseDevice;
}

namespace ui {
namespace tabs {

class BaseTab;

namespace tabhelper {

/**
 * Returns the fitting tab for the given device, by checking the device type.
 *
 * @param[in] session The reference to the actual SmuView session
 * @param[in] device The base device
 * @param[in] parent The parent of the tab (normaly nullptr)
 *
 * @return The tab for the device
 */
BaseTab *get_tab_for_device(Session &session,
	shared_ptr<sv::devices::BaseDevice> device, QWidget *parent = nullptr);

} // namespace tabhelper
} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_TABHELPER_HPP
