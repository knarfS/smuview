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

#include <memory>
#include <mutex>
#include <string>

#include <QDebug>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QVariant>

#include "devicetreemodel.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/ui/devices/devicetree/treeitem.hpp"

using std::shared_ptr;
using std::string;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace devices {
namespace devicetree {

DeviceTreeModel::DeviceTreeModel(const Session &session,
		bool is_device_checkable, bool is_channel_group_checkable,
		bool is_channel_checkable, bool is_signal_checkable,
		bool is_configurable_checkable, bool is_config_key_checkable,
		QObject *parent) :
	QStandardItemModel(parent),
	session_(session),
	is_device_checkable_(is_device_checkable),
	is_channel_group_checkable_(is_channel_group_checkable),
	is_channel_checkable_(is_channel_checkable),
	is_signal_checkable_(is_signal_checkable),
	is_configurable_checkable_(is_configurable_checkable),
	is_config_key_checkable_(is_config_key_checkable)
{
	this->itemPrototype();
	setup_model();
}

void DeviceTreeModel::setup_model()
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);

	setSortRole(DeviceTreeModel::SortRole);

	connect(
		&session_, SIGNAL(device_added(shared_ptr<sv::devices::BaseDevice>)),
		this, SLOT(on_device_added(shared_ptr<sv::devices::BaseDevice>)));
	connect(
		&session_, SIGNAL(device_removed(shared_ptr<sv::devices::BaseDevice>)),
		this, SLOT(on_device_removed(shared_ptr<sv::devices::BaseDevice>)));

	for (const auto &device_pair : session_.devices()) {
		shared_ptr<sv::devices::BaseDevice> device = device_pair.second;
		add_device(device);
	}

	invisibleRootItem()->sortChildren(0);
}

void DeviceTreeModel::add_device(shared_ptr<sv::devices::BaseDevice> device)
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);

	/*
	this->setFirstItemColumnSpanned(device_item, true);
	*/

	// Look for existing device
	TreeItem *device_item = find_device(device);
	if (!device_item) {
		beginInsertRows(invisibleRootItem()->index(),
			invisibleRootItem()->rowCount(), invisibleRootItem()->rowCount()+1);
		device_item = new TreeItem(TreeItemType::DeviceItem);
		device_item->setText(device->full_name());
		device_item->setData(QVariant::fromValue(device), DeviceTreeModel::DataRole);
		device_item->setData(device->full_name(), DeviceTreeModel::SortRole);
		device_item->setCheckable(is_device_checkable_);
		invisibleRootItem()->appendRow(device_item);
		endInsertRows();

		invisibleRootItem()->sortChildren(0);

		connect(
			device.get(),
			SIGNAL(channel_added(shared_ptr<sv::channels::BaseChannel>)),
			this, SLOT(on_channel_added(shared_ptr<sv::channels::BaseChannel>)));
	}

	// Channels and ChannelGroups
	for (const auto &channel_pair : device->channel_map()) {
		add_channel(channel_pair.second,
			channel_pair.second->channel_group_names(), device_item);
	}

	// Configurbles and ConfigKeys
	/* TODO
	auto configurables_map = device->configurable();
	*/
}

void DeviceTreeModel::add_channel(shared_ptr<channels::BaseChannel> channel,
	set<string> channel_group_names, TreeItem *parent_item)
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);

	// Find existing channel in all channel groups
	if (!find_channel(channel, channel->channel_group_names(), parent_item)) {
		connect(
			channel.get(),
			SIGNAL(signal_added(shared_ptr<sv::data::BaseSignal>)),
			this, SLOT(on_signal_added(shared_ptr<sv::data::BaseSignal>)));
	}

	for (const auto &chg_name : channel_group_names) {
		TreeItem *new_parent_item = nullptr;
		if (!chg_name.empty()) {
			// Look for already existing channel group
			new_parent_item = find_channel_group(chg_name, parent_item);
			if (!new_parent_item) {
				QString chg_name_qstr = QString::fromStdString(chg_name);
				beginInsertRows(parent_item->index(),
					parent_item->rowCount(), parent_item->rowCount()+1);
				new_parent_item = new TreeItem(TreeItemType::ChannelGroupItem);
				new_parent_item->setText(chg_name_qstr);
				new_parent_item->setData(chg_name_qstr, DeviceTreeModel::DataRole);
				new_parent_item->setData(chg_name_qstr, DeviceTreeModel::SortRole);
				new_parent_item->setCheckable(is_channel_group_checkable_);
				parent_item->appendRow(new_parent_item);
				endInsertRows();

				parent_item->sortChildren(0);
			}
		}
		else
			new_parent_item = parent_item;

		// Look for existing channel
		set<string> chg_names { chg_name };
		TreeItem *channel_item = find_channel(channel, chg_names, parent_item);
		if (!channel_item) {
			beginInsertRows(new_parent_item->index(),
				new_parent_item->rowCount(), new_parent_item->rowCount()+1);
			channel_item = new TreeItem(TreeItemType::ChannelItem);
			channel_item->setText(QString::fromStdString(channel->name()));
			channel_item->setData(QVariant::fromValue(channel), DeviceTreeModel::DataRole);
			channel_item->setData(channel->index(), DeviceTreeModel::SortRole);
			channel_item->setCheckable(is_channel_checkable_);
			new_parent_item->appendRow(channel_item);
			endInsertRows();

			new_parent_item->sortChildren(0);
		}

		// Signals
		for (const auto &signal_pair : channel->signal_map()) {
			for (const auto &signal : signal_pair.second) {
				add_signal(signal, channel_item);
			}
		}
	}
}

void DeviceTreeModel::add_signal(shared_ptr<sv::data::BaseSignal> signal,
	TreeItem *parent_item)
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);

	// Look for existing signal
	TreeItem *signal_item = find_signal(signal, parent_item);
	if (!signal_item) {
		beginInsertRows(parent_item->index(),
			parent_item->rowCount(), parent_item->rowCount()+1);
		signal_item = new TreeItem(TreeItemType::SignalItem);
		signal_item->setText(signal->name());
		signal_item->setData(QVariant::fromValue(signal), DeviceTreeModel::DataRole);
		signal_item->setData(signal->name(), DeviceTreeModel::SortRole);
		signal_item->setCheckable(is_signal_checkable_);
		parent_item->appendRow(signal_item);
		endInsertRows();

		parent_item->sortChildren(0);
	}
}

TreeItem *DeviceTreeModel::find_device(
	shared_ptr<sv::devices::BaseDevice> device) const
{
	for (int i=0; i<invisibleRootItem()->rowCount(); ++i) {
		auto child = invisibleRootItem()->child(i);
		if (child->type() != (int)TreeItemType::DeviceItem)
			continue;

		if (device.get() == child->data(DeviceTreeModel::DataRole).
				value<shared_ptr<sv::devices::BaseDevice>>().get())
			return (TreeItem *)child;
	}
	return nullptr;
}

TreeItem *DeviceTreeModel::find_channel_group(string channel_group_name,
	TreeItem *parent_item) const
{
	for (int i=0; i<parent_item->rowCount(); ++i) {
		auto child = parent_item->child(i);
		if (child->type() != (int)TreeItemType::ChannelGroupItem)
			continue;

		QString chg_name_qstr = QString::fromStdString(channel_group_name);
		if (chg_name_qstr == child->data(DeviceTreeModel::DataRole).toString())
			return (TreeItem *)child;
	}
	return nullptr;
}

TreeItem *DeviceTreeModel::find_channel(
	shared_ptr<sv::channels::BaseChannel> channel,
	set<string> channel_group_names, TreeItem *parent_item) const
{
	vector<TreeItem *> channels;
	TreeItem *new_parent_item;
	for (const auto &chg_name : channel_group_names) {
		if (!chg_name.empty()) {
			new_parent_item = find_channel_group(chg_name, parent_item);
			if (!new_parent_item)
				continue;
		}
		else
			new_parent_item = parent_item;

		for (int i=0; i<new_parent_item->rowCount(); ++i) {
			auto child = new_parent_item->child(i);
			if (child->type() != (int)TreeItemType::ChannelItem)
				continue;

			if (channel.get() == child->data(DeviceTreeModel::DataRole).
					value<shared_ptr<sv::channels::BaseChannel>>().get())
				return (TreeItem *)child;
		}
	}
	return nullptr;
}

TreeItem *DeviceTreeModel::find_signal (
	shared_ptr<sv::data::BaseSignal> signal, TreeItem *parent_item) const
{
	for (int i=0; i<parent_item->rowCount(); ++i) {
		auto child = parent_item->child(i);
		if (child->type() != (int)TreeItemType::SignalItem)
			continue;

		if (signal.get() == child->data(DeviceTreeModel::DataRole).
				value<shared_ptr<sv::data::BaseSignal>>().get())
			return (TreeItem *)child;
	}
	return nullptr;
}

void DeviceTreeModel::on_device_added(shared_ptr<sv::devices::BaseDevice> device)
{
	add_device(device);
}

void DeviceTreeModel::on_device_removed(shared_ptr<sv::devices::BaseDevice> device)
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);

	TreeItem *item = find_device(device);
	if (item) {
		removeRow(item->row(), invisibleRootItem()->index());
	}
}

void DeviceTreeModel::on_channel_added(shared_ptr<channels::BaseChannel> channel)
{
	shared_ptr<sv::devices::BaseDevice> device = channel->parent_device();
	// Device must exist
	TreeItem *device_item = find_device(device);
	add_channel(channel, channel->channel_group_names(), device_item);
}

void DeviceTreeModel::on_channel_removed(shared_ptr<channels::BaseChannel> channel)
{
	(void)channel;
	std::lock_guard<std::recursive_mutex> lock(mutex_);
}

void DeviceTreeModel::on_signal_added(shared_ptr<sv::data::BaseSignal> signal)
{
	shared_ptr<sv::channels::BaseChannel> channel = signal->parent_channel();
	on_channel_added(channel);
}

void DeviceTreeModel::on_signal_removed(shared_ptr<sv::data::BaseSignal> signal)
{
	(void)signal;
	std::lock_guard<std::recursive_mutex> lock(mutex_);
}

} // namespace devicetree
} // namespace devices
} // namespace ui
} // namespace sv
