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

#include "flowtab.hpp"
#include "src/session.hpp"
#include "src/ui/tabs/basetab.hpp"
#include "src/ui/views/flowview.hpp"

namespace sv {
namespace ui {
namespace tabs {

FlowTab::FlowTab(Session &session, QMainWindow *parent) :
	BaseTab(session, parent)
{
	setup_ui();
}

void FlowTab::setup_ui()
{
	auto view = new views::FlowView(session_);
	add_view(view, Qt::RightDockWidgetArea);
}

} // namespace tabs
} // namespace ui
} // namespace sv
