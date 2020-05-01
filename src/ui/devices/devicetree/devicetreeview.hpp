/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DEVICES_DEVICETREE_DEVICETREEVIEW_HPP
#define UI_DEVICES_DEVICETREE_DEVICETREEVIEW_HPP

#include <memory>
#include <vector>

#include <QModelIndex>
#include <QStandardItem>
#include <QTreeView>

using std::shared_ptr;
using std::vector;

namespace sv {

class Session;

namespace channels {
class BaseChannel;
}
namespace data {
class BaseSignal;
}
namespace devices {
class BaseDevice;
}

namespace ui {
namespace devices {
namespace devicetree {

class DeviceTreeModel;
class TreeItem;

class DeviceTreeView : public QTreeView
{
	Q_OBJECT

public:
	explicit DeviceTreeView(const Session &session,
		bool is_device_checkable, bool is_channel_group_checkable,
		bool is_channel_checkable, bool is_signal_checkable,
		bool is_configurable_checkable, bool is_config_key_checkable,
		bool show_configurable, bool is_auto_expand, QWidget *parent = nullptr);

	void select_device(shared_ptr<sv::devices::BaseDevice> device);

	void select_item(TreeItem *item);
	TreeItem *selected_item() const;

	void check_channels(const vector<shared_ptr<sv::channels::BaseChannel>> channels);
	vector<shared_ptr<sv::channels::BaseChannel>> checked_channels() const;
	void check_signals(const vector<shared_ptr<sv::data::BaseSignal>> signals);
	vector<shared_ptr<sv::data::BaseSignal>> checked_signals() const;

	void expand_device(shared_ptr<sv::devices::BaseDevice> device);

private:
	void setup_ui();
	void expand_recursive(QStandardItem *item);

	const Session &session_;
	bool is_device_checkable_;
	bool is_channel_group_checkable_;
	bool is_channel_checkable_;
	bool is_signal_checkable_;
	bool is_configurable_checkable_;
	bool is_config_key_checkable_;
	bool show_configurable_;
	bool is_auto_expand_;
	DeviceTreeModel *tree_model_;

private Q_SLOTS:
	void on_rows_inserted(const QModelIndex &model_index, int first, int last);

};

} // namespace devicetree
} // namespace devices
} // namespace ui
} // namespace sv

#endif // UI_DEVICES_DEVICETREE_DEVICETREEVIEW_HPP
