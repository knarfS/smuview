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

#include <QIcon>
#include <QStandardItem>

#include "treeitem.hpp"

namespace sv {
namespace ui {
namespace devices {
namespace devicetree {

TreeItem::TreeItem(TreeItemType type) :
	QStandardItem(),
	type_(type)
{
	if (type == TreeItemType::DeviceItem) {
		setIcon(QIcon(":/icons/smuview.png"));
	}
	else if (type == TreeItemType::ChannelGroupItem) {
		setIcon(QIcon::fromTheme("document-open-folder",
			QIcon(":/icons/document-open-folder.png")));
	}
	else if (type == TreeItemType::ChannelItem) {
		setIcon(QIcon::fromTheme("office-chart-area",
			QIcon(":/icons/office-chart-area.png")));
	}
	else if (type == TreeItemType::SignalItem) {
		setIcon(QIcon::fromTheme("office-chart-line",
			QIcon(":/icons/office-chart-line.png")));
	}
	else if (type == TreeItemType::ConfigurableItem) {
		setIcon(QIcon(":/icons/smuview.png"));
	}
	else if (type == TreeItemType::ConfigKeyItem) {
		setIcon(QIcon(":/icons/smuview.png"));
	}
}

int TreeItem::type() const
{
	return (int)type_;
}

} // namespace devicetree
} // namespace devices
} // namespace ui
} // namespace sv
