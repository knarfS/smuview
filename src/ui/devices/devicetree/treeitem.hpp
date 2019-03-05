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

#ifndef UI_DEVICES_DEVICETREE_TREEITEM_HPP
#define UI_DEVICES_DEVICETREE_TREEITEM_HPP

#include <QStandardItem>
#include <QIcon>

namespace sv {
namespace ui {
namespace devices {
namespace devicetree {

enum class TreeItemType {
	DeviceItem = 1001,
	ChannelGroupItem = 1002,
	ChannelItem = 1003,
	SignalItem = 1004,
	ConfigurableItem = 1005,
	ConfigKeyItem = 1006,
};

class TreeItem : public QStandardItem
{

public:
	TreeItem(TreeItemType type);

	int type() const override;

protected:
	TreeItemType type_;

};

} // namespace devicetree
} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_DEVICES_DEVICETREE_TREEITEM_HPP
