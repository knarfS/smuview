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

#ifndef UI_DEVICES_DEVICETREE_DEVICETREEMODEL_HPP
#define UI_DEVICES_DEVICETREE_DEVICETREEMODEL_HPP

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <QStandardItem>
#include <QStandardItemModel>

#include "src/devices/basedevice.hpp"

using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {

class Session;

namespace ui {
namespace devices {
namespace devicetree {

class TreeItem;

class DeviceTreeModel : public QStandardItemModel
{
	Q_OBJECT

public:
	explicit DeviceTreeModel(const Session &session,
		bool is_device_checkable, bool is_channel_group_checkable,
		bool is_channel_checkable, bool is_signal_checkable,
		bool is_configurable_checkable, bool is_config_key_checkable,
		QObject *parent = 0);

	TreeItem *find_device(shared_ptr<sv::devices::BaseDevice> device) const;

	const static int DataRole = Qt::UserRole + 1;
	const static int SortRole = Qt::UserRole + 2;

private:
	void setup_model();
	void add_device(shared_ptr<sv::devices::BaseDevice> device);
	void add_channel(shared_ptr<channels::BaseChannel> channel,
		set<string> channel_group_names, TreeItem *parent_item);
	void add_signal(shared_ptr<sv::data::BaseSignal> signal,
		TreeItem *parent_item);
	TreeItem *find_channel_group(string channel_group_name,
		TreeItem *parent_item) const;
	TreeItem *find_channel(shared_ptr<sv::channels::BaseChannel> channel,
		set<string> channel_group_names, TreeItem *parent_item) const;
	TreeItem *find_signal(shared_ptr<sv::data::BaseSignal> signal,
		TreeItem *parent_item) const;

	const Session &session_;
	bool is_device_checkable_;
	bool is_channel_group_checkable_;
	bool is_channel_checkable_;
	bool is_signal_checkable_;
	bool is_configurable_checkable_;
	bool is_config_key_checkable_;
	std::recursive_mutex mutex_;

private Q_SLOTS:
	void on_device_added(shared_ptr<sv::devices::BaseDevice> device);
	void on_device_removed(shared_ptr<sv::devices::BaseDevice> device);
	void on_channel_added(shared_ptr<sv::channels::BaseChannel> channel);
	void on_channel_removed(shared_ptr<sv::channels::BaseChannel> channel);
	void on_signal_added(shared_ptr<sv::data::BaseSignal> signal);
	void on_signal_removed(shared_ptr<sv::data::BaseSignal> signal);

};

} // namespace devicetree
} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_DEVICES_DEVICETREE_DEVICETREEMODEL_HPP
