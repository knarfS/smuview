/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <map>
#include <string>
#include <utility>

#include <QDebug>
#include <QHeaderView>
#include <QTreeView>

#include "signaltree.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"

using std::dynamic_pointer_cast;
using std::make_pair;
using std::make_shared;
using std::map;
using std::static_pointer_cast;
using std::string;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace devices {

SignalTree::SignalTree(const Session &session,
		bool show_signals, bool selectable, bool multiselect,
		shared_ptr<sv::devices::BaseDevice> selected_device,
		QWidget *parent) :
	QTreeWidget(parent),
	session_(session),
	selected_device_(selected_device),
	show_signals_(show_signals),
	selectable_(selectable),
	multiselect_(multiselect)
{
	setup_ui();
}

vector<shared_ptr<channels::BaseChannel>> SignalTree::selected_channels()
{
	vector<shared_ptr<channels::BaseChannel>> channels;
	auto items = this->checked_items();
	for (const auto &item : items) {
		QVariant data = item->data(0, Qt::UserRole);
		if (data.isNull())
			continue;

		auto channel = data.value<shared_ptr<channels::BaseChannel>>();
		if (channel)
			channels.push_back(channel);
	}
	return channels;
}

vector<shared_ptr<sv::data::BaseSignal>> SignalTree::selected_signals()
{
	vector<shared_ptr<sv::data::BaseSignal>> signals;
	for (const auto &item : this->checked_items()) {
		QVariant data = item->data(0, Qt::UserRole);
		if (data.isNull())
			continue;

		auto signal = data.value<shared_ptr<sv::data::BaseSignal>>();
		if (signal)
			signals.push_back(signal);
	}
	return signals;
}

void SignalTree::setup_ui()
{
	this->setColumnCount(1); // 2
	this->header()->close();
	if (multiselect_)
		this->setSelectionMode(QTreeView::MultiSelection);

	map<string, shared_ptr<sv::devices::BaseDevice>> devices;
	if (!selected_device_)
		devices = session_.devices();
	else if (selected_device_)
		devices.insert(make_pair(selected_device_->id(), selected_device_));

	for (const auto &device : devices) {
		bool expanded = true;
		/*
		bool expanded = false;
		if (selected_device_ && selected_device_.get() == device.second.get())
			expanded = true;
		*/
		/* TODO
		for (const auto &sel_signal : selected_signals_) {
			if (sel_signal.get() == signal.get()) {
				signal_item->setSelected(true);
				break;
			}
		}
		*/

		add_device(device.second, expanded);
	}

	// Resize all columns
	this->resizeColumnToContents(0);
	this->resizeColumnToContents(1);
}

void SignalTree::add_device(shared_ptr<sv::devices::BaseDevice> device,
	bool expanded)
{
	QTreeWidgetItem *device_item = new QTreeWidgetItem();
	device_item->setIcon(0, QIcon(":/icons/smuview.png"));
	device_item->setText(0, device->full_name());
	this->addTopLevelItem(device_item);
	this->setFirstItemColumnSpanned(device_item, true);

	device_map_.insert(make_pair(device, device_item));

	auto chg_name_channels_map = device->channel_group_name_map();
	for (const auto &chg_name_channels_pair : chg_name_channels_map) {
		for (const auto &channel : chg_name_channels_pair.second) {
			add_channel(channel, chg_name_channels_pair.first,
				expanded, device_item);
		}
	}
	device_item->setExpanded(expanded);

	connect(
		device.get(), SIGNAL(channel_added(shared_ptr<channels::BaseChannel>)),
		this, SLOT(on_channel_added(shared_ptr<channels::BaseChannel>)));

	return;
}

void SignalTree::add_channel(shared_ptr<channels::BaseChannel> channel,
	QString channel_group_name, bool expanded, QTreeWidgetItem *parent)
{
	QTreeWidgetItem *chg_item;
	if (channel_group_name.size() > 0) {
		chg_item = this->find_item(parent, channel_group_name);
		if (!chg_item) {
			// Channel is in a channel group, add group first
			chg_item = new QTreeWidgetItem();
			chg_item->setIcon(0,
				QIcon::fromTheme("document-open-folder",
				QIcon(":/icons/document-open-folder.png")));
			chg_item->setText(0, channel_group_name);
			parent->addChild(chg_item);
		}

		parent = chg_item;
	}

	// Channel already in the tree?
	QTreeWidgetItem *ch_item;
	if (channel_map_.count(channel) != 0)
		ch_item = channel_map_[channel];
	else {
		ch_item = new QTreeWidgetItem();
		if (selectable_) {
			ch_item->setFlags(ch_item->flags() | Qt::ItemIsUserCheckable); // | Qt::ItemIsSelectable
			ch_item->setCheckState(0, Qt::Unchecked);
		}

		channel_map_.insert(make_pair(channel, ch_item));

		if (show_signals_) {
			for (const auto &signal_pair : channel->signal_map())
				add_signal(signal_pair.second, ch_item);
		}

		connect(
			channel.get(), SIGNAL(signal_added(shared_ptr<sv::data::BaseSignal>)),
			this, SLOT(on_signal_added(shared_ptr<sv::data::BaseSignal>)));
	}
	ch_item->setIcon(0,
		QIcon::fromTheme("office-chart-area",
		QIcon(":/icons/office-chart-area.png")));
	ch_item->setText(0, channel->name());
	ch_item->setData(0, Qt::UserRole, QVariant::fromValue(channel));
	parent->addChild(ch_item);

	ch_item->setExpanded(expanded);
	if (channel_group_name.size() > 0)
		chg_item->setExpanded(expanded);
}

void SignalTree::add_signal(shared_ptr<sv::data::BaseSignal> signal,
	QTreeWidgetItem *parent)
{
	QTreeWidgetItem *signal_item = new QTreeWidgetItem();
	if (selectable_) {
		signal_item->setFlags(signal_item->flags() | Qt::ItemIsUserCheckable); // | Qt::ItemIsSelectable
		signal_item->setCheckState(0, Qt::Unchecked);
	}
	signal_item->setIcon(0,
		QIcon::fromTheme("office-chart-line",
		QIcon(":/icons/office-chart-line.png")));
	signal_item->setText(0, signal->name());
	signal_item->setData(0, Qt::UserRole, QVariant::fromValue(signal));
	parent->addChild(signal_item);
}

vector<const QTreeWidgetItem *> SignalTree::checked_items()
{
	vector<const QTreeWidgetItem *> items;
	for (int i = 0; i < this->topLevelItemCount(); i++) {
		const QTreeWidgetItem *item = this->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked)
			items.push_back(item);

		vector<const QTreeWidgetItem *> child_items =
			checked_items_recursiv(item);
		items.insert(items.end(), child_items.begin(), child_items.end());

	}
	return items;
}

vector<const QTreeWidgetItem *> SignalTree::checked_items_recursiv(
	const QTreeWidgetItem * item)
{
	vector<const QTreeWidgetItem *> items;
	for (int i = 0; i < item->childCount(); i++) {
		const QTreeWidgetItem *child_item = item->child(i);
		if (child_item->checkState(0) == Qt::Checked)
			items.push_back(child_item);

		vector<const QTreeWidgetItem *> child_items =
			checked_items_recursiv(child_item);
		items.insert(items.end(), child_items.begin(), child_items.end());
	}
	return items;
}

QTreeWidgetItem *SignalTree::find_item(QTreeWidgetItem *parent, QString name)
{
	for (int i = 0; i < parent->childCount(); ++i) {
		auto item = parent->child(i);
		if (item->text(0) == name)
			return item;
	}
	return NULL;
}

void SignalTree::on_device_added(shared_ptr<sv::devices::BaseDevice> device)
{
	add_device(device, true);
}

void SignalTree::on_device_removed()
{
}

void SignalTree::on_channel_added(shared_ptr<channels::BaseChannel> channel)
{
	auto parent_item = device_map_[channel->parent_device()];
	for (const auto &chg_name : channel->channel_group_names()) {
		add_channel(channel, chg_name, true, parent_item);
	}
}

void SignalTree::on_channel_removed()
{
}

void SignalTree::on_signal_added(shared_ptr<sv::data::BaseSignal> signal)
{
	auto parent_item = channel_map_[signal->parent_channel()];
	add_signal(signal, parent_item);
}

void SignalTree::on_signal_removed()
{
}

} // namespace devices
} // namespace ui
} // namespace sv
