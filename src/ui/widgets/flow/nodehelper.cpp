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

#include <QString>
#include <QWidget>
#include <Node>
#include <NodeDataModel>

#include "nodehelper.hpp"
#include "src/session.hpp"
#include "src/ui/widgets/flow/doubleconfigsinkpopup.hpp"
#include "src/ui/widgets/flow/rampsourcepopup.hpp"
#include "src/ui/widgets/popup.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace flow {
namespace nodehelper {

Popup *get_popup_for_node(const Session &session,
	QtNodes::Node &node, QWidget *parent)
{
	QString node_name = node.nodeDataModel()->name();

	if (node_name == "Start")
		return NULL;
	else if (node_name == "NumberSource")
		return NULL;
	else if (node_name == "RampSource")
		return new RampSourcePopup(node, parent);
	else if (node_name == "DoubleConfigKeySink")
		return new DoubleConfigSinkPopup(session, node, parent);
	else
		return NULL;
}

} // namespace nodehelper
} // namespace flow
} // namespace widgets
} // namespace ui
} // namespace sv
