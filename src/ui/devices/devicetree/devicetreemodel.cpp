/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>
#include <mutex>
#include <set>
#include <string>

#include <QDebug>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QVariant>

#include "devicetreemodel.hpp"
#include "src/session.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/channels/basechannel.hpp"
#include "src/ui/devices/devicetree/treeitem.hpp"

using std::set;
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
		bool show_configurable, QObject *parent) :
	QStandardItemModel(parent),
	session_(session),
	is_device_checkable_(is_device_checkable),
	is_channel_group_checkable_(is_channel_group_checkable),
	is_channel_checkable_(is_channel_checkable),
	is_signal_checkable_(is_signal_checkable),
	is_configurable_checkable_(is_configurable_checkable),
	is_config_key_checkable_(is_config_key_checkable),
	show_configurable_(show_configurable)
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

	for (const auto &device_pair : session_.device_map()) {
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
		device_item->setEditable(false);
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

	// Configurables and ConfigKeys
	for (const auto &configurable_pair : device->configurable_map()) {
		add_configurable(configurable_pair.second, device_item);
	}
}

TreeItem *DeviceTreeModel::add_channel_group(const string &channel_group_name,
	TreeItem *device_item)
{
	if (channel_group_name.empty())
		return device_item;

	// Look for already existing channel group
	TreeItem *chg_item = find_channel_group(channel_group_name, device_item);
	if (chg_item)
		return chg_item;

	std::lock_guard<std::recursive_mutex> lock(mutex_);

	QString chg_name_qstr = QString::fromStdString(channel_group_name);
	beginInsertRows(device_item->index(),
		device_item->rowCount(), device_item->rowCount()+1);
	chg_item = new TreeItem(TreeItemType::ChannelGroupItem);
	chg_item->setText(chg_name_qstr);
	chg_item->setData(chg_name_qstr, DeviceTreeModel::DataRole);
	chg_item->setData(chg_name_qstr, DeviceTreeModel::SortRole);
	chg_item->setCheckable(is_channel_group_checkable_);
	chg_item->setEditable(false);
	device_item->appendRow(chg_item);
	endInsertRows();

	device_item->sortChildren(0);

	return chg_item;
}

void DeviceTreeModel::add_channel(shared_ptr<channels::BaseChannel> channel,
	const set<string> &channel_group_names, TreeItem *parent_item)
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);

	// Find existing channel in all channel groups
	// NOLINTNEXTLINE(readability-implicit-bool-conversion)
	if (!find_channel(channel, channel->channel_group_names(), parent_item)) {
		connect(
			channel.get(),
			SIGNAL(signal_added(shared_ptr<sv::data::BaseSignal>)),
			this, SLOT(on_signal_added(shared_ptr<sv::data::BaseSignal>)));
	}

	for (const auto &chg_name : channel_group_names) {
		TreeItem *new_parent_item = add_channel_group(chg_name, parent_item);

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
			channel_item->setEditable(false);
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
		signal_item->setText(signal->display_name());
		signal_item->setData(QVariant::fromValue(signal), DeviceTreeModel::DataRole);
		signal_item->setData(signal->display_name(), DeviceTreeModel::SortRole); // TODO: signal->index()
		signal_item->setCheckable(is_signal_checkable_);
		signal_item->setEditable(false);
		parent_item->appendRow(signal_item);
		endInsertRows();

		parent_item->sortChildren(0);
	}
}

void DeviceTreeModel::add_configurable(
	shared_ptr<sv::devices::Configurable> configurable, TreeItem *device_item)
{
	if (!show_configurable_)
		return;

	std::lock_guard<std::recursive_mutex> lock(mutex_);

	// Find existing configurable in device and all channel groups
	TreeItem *conf_item = find_configurable(configurable, device_item);
	if (!conf_item) {
		TreeItem *new_parent_item = add_channel_group(
			configurable->name(), device_item);

		// Add configurable item
		beginInsertRows(new_parent_item->index(),
			new_parent_item->rowCount(), new_parent_item->rowCount()+1);
		conf_item = new TreeItem(TreeItemType::ConfigurableItem);
		conf_item->setText(configurable->display_name());
		conf_item->setData(QVariant::fromValue(configurable), DeviceTreeModel::DataRole);
		conf_item->setData(configurable->index(), DeviceTreeModel::SortRole);
		conf_item->setCheckable(false);
		conf_item->setEditable(false);
		new_parent_item->appendRow(conf_item);
		endInsertRows();

		new_parent_item->sortChildren(0);
	}

	// ConfigKeys
	for (const auto &property_pair : configurable->property_map()) {
		add_property(property_pair.second, conf_item);
	}
}

void DeviceTreeModel::add_property(
	shared_ptr<sv::data::properties::BaseProperty> property,
	TreeItem *configurable_item)
{
	if (!show_configurable_)
		return;

	std::lock_guard<std::recursive_mutex> lock(mutex_);

	// Look for existing property
	TreeItem *property_item = find_property(property, configurable_item);
	if (!property_item) {
		beginInsertRows(configurable_item->index(),
			configurable_item->rowCount(), configurable_item->rowCount()+1);
		property_item = new TreeItem(TreeItemType::PropertyItem);
		property_item->setText(property->display_name());
		property_item->setData(QVariant::fromValue(property), DeviceTreeModel::DataRole);
		property_item->setData(property->display_name(), DeviceTreeModel::SortRole);
		property_item->setCheckable(is_signal_checkable_);
		property_item->setEditable(false);
		configurable_item->appendRow(property_item);
		endInsertRows();

		configurable_item->sortChildren(0);
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
			return static_cast<TreeItem *>(child);
	}
	return nullptr;
}

TreeItem *DeviceTreeModel::find_channel_group(const string &channel_group_name,
	TreeItem *parent_item) const
{
	for (int i=0; i<parent_item->rowCount(); ++i) {
		auto child = parent_item->child(i);
		if (child->type() != (int)TreeItemType::ChannelGroupItem)
			continue;

		QString chg_name_qstr = QString::fromStdString(channel_group_name);
		if (chg_name_qstr == child->data(DeviceTreeModel::DataRole).toString())
			return static_cast<TreeItem *>(child);
	}
	return nullptr;
}

TreeItem *DeviceTreeModel::find_channel(
	shared_ptr<sv::channels::BaseChannel> channel,
	const set<string> &channel_group_names, TreeItem *parent_item) const
{
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
				return static_cast<TreeItem *>(child);
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
			return static_cast<TreeItem *>(child);
	}
	return nullptr;
}

TreeItem *DeviceTreeModel::find_configurable(
	shared_ptr<sv::devices::Configurable> configurable,
	TreeItem *device_item) const
{
	TreeItem *new_parent_item;
	if (!configurable->name().empty()) {
		new_parent_item = find_channel_group(configurable->name(), device_item);
		if (!new_parent_item)
			return nullptr;
	}
	else {
		new_parent_item = device_item;
	}

	for (int i=0; i<new_parent_item->rowCount(); ++i) {
		auto child = new_parent_item->child(i);
		if (child->type() != (int)TreeItemType::ConfigurableItem)
			continue;

		if (configurable.get() == child->data(DeviceTreeModel::DataRole).
				value<shared_ptr<sv::devices::Configurable>>().get())
			return static_cast<TreeItem *>(child);
	}
	return nullptr;
}

TreeItem *DeviceTreeModel::find_property(
	shared_ptr<sv::data::properties::BaseProperty> property,
	TreeItem *configurable_item) const
{
	for (int i=0; i<configurable_item->rowCount(); ++i) {
		auto child = configurable_item->child(i);
		if (child->type() != (int)TreeItemType::PropertyItem)
			continue;

		if (property.get() == child->data(DeviceTreeModel::DataRole).
				value<shared_ptr<sv::data::properties::BaseProperty>>().get())
			return static_cast<TreeItem *>(child);
	}
	return nullptr;
}

void DeviceTreeModel::on_device_added(
	shared_ptr<sv::devices::BaseDevice> device)
{
	add_device(device);
}

void DeviceTreeModel::on_device_removed(
	shared_ptr<sv::devices::BaseDevice> device)
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);

	TreeItem *item = find_device(device);
	if (item) {
		removeRow(item->row(), invisibleRootItem()->index());
	}
}

void DeviceTreeModel::on_channel_added(
	shared_ptr<channels::BaseChannel> channel)
{
	shared_ptr<sv::devices::BaseDevice> device = channel->parent_device();
	// Device must exist
	TreeItem *device_item = find_device(device);
	add_channel(channel, channel->channel_group_names(), device_item);
}

void DeviceTreeModel::on_channel_removed(
	shared_ptr<channels::BaseChannel> channel)
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
