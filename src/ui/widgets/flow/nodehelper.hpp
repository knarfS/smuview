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

#ifndef UI_WIDGETS_FLOW_NODEHELPER_HPP
#define UI_WIDGETS_FLOW_NODEHELPER_HPP

#include <QWidget>
#include <Node>

namespace sv {

class Session;

namespace ui {
namespace widgets {

class Popup;

namespace flow {
namespace nodehelper {

/**
 * Returns the Popup widget for the given node.
 *
 * @param[in] session The smuview session
 * @param[in] node The node
 *
 * @return The Popup widget for the node.
 */
Popup *get_popup_for_node(const Session &session,
	QtNodes::Node &node, QWidget *parent);

} // namespace nodehelper
} // namespace flow
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_FLOW_NODEHELPER_HPP
