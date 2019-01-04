/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_TABS_FLOWTAB_HPP
#define UI_TABS_FLOWTAB_HPP

#include <QObject>

#include "src/ui/tabs/basetab.hpp"

namespace sv {

class Session;

namespace ui {
namespace tabs {

class FlowTab : public BaseTab
{
	Q_OBJECT

public:
	FlowTab(Session &session,  QMainWindow *parent);

private:
	void setup_ui();

};

} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_FLOWTAB_HPP
