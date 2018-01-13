/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <unordered_set>

#include <QDebug>
#include <QTreeView>

#include "signaltree.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/channel.hpp"
#include "src/devices/device.hpp"
#include "src/devices/hardwaredevice.hpp"

using std::dynamic_pointer_cast;
using std::static_pointer_cast;
using std::unordered_set;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace widgets {

SignalTree::SignalTree(const Session &session, bool is_show_signals,
		shared_ptr<devices::Device> selected_device,
		QWidget *parent) :
	QTreeWidget(parent),
	session_(session),
	selected_device_(selected_device),
	is_show_signals_(is_show_signals)
{
	setup_ui();
}

vector<shared_ptr<devices::Channel>> SignalTree::selected_channels()
{
	vector<shared_ptr<devices::Channel>> channels;
	auto items = this->checked_items();
	for (auto item : items) {
		QVariant data = item->data(0, Qt::UserRole);
		if (data.isNull())
			continue;

		/*
		shared_ptr<devices::Channel> channel =
			dynamic_pointer_cast<devices::Channel>(data.value);
		*/
		shared_ptr<devices::Channel> channel =
			data.value<shared_ptr<devices::Channel>>();

		if (channel)
			channels.push_back(channel);
	}
	return channels;
}

vector<shared_ptr<data::BaseSignal>> SignalTree::selected_signals()
{
	vector<shared_ptr<data::BaseSignal>> signals;
	for (auto item : this->selectedItems()) {
		QVariant data = item->data(0, Qt::UserRole);
		if (data.isNull())
			continue;

		/*
		shared_ptr<data::BaseSignal> signal =
			dynamic_pointer_cast<data::BaseSignal>(data.value);
		*/
		shared_ptr<data::BaseSignal> signal =
			data.value<shared_ptr<data::BaseSignal>>();

		if (signal)
			signals.push_back(signal);
	}
	return signals;
}

void SignalTree::setup_ui()
{
	this->setColumnCount(1);
	this->setSelectionMode(QTreeView::MultiSelection);

	unordered_set<shared_ptr<devices::Device>> devices;
	if (!selected_device_)
		devices = session_.devices();
	else
		devices.insert(selected_device_);

	for (auto device : devices) {
		bool expanded = false;
		if (selected_device_ && selected_device_.get() == device.get())
			expanded = true;
		/* TODO
		for (auto sel_signal : selected_signals_) {
			if (sel_signal.get() == signal.get()) {
				signal_item->setSelected(true);
				break;
			}
		}
		*/

		// TODO: Move channel stuff from HardwareDevice to Device
		shared_ptr<devices::HardwareDevice> hw_device =
			static_pointer_cast<devices::HardwareDevice>(device);

		// Tree root
		QTreeWidgetItem *device_item = new QTreeWidgetItem();
		device_item->setFlags(
			device_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
		device_item->setCheckState(0, Qt::Checked);
		device_item->setIcon(0, QIcon(":/icon/smuview.ico"));
		device_item->setText(0, hw_device->full_name());
		this->addTopLevelItem(device_item);

		auto chg_name_channels_map = hw_device->channel_group_name_map();
		for (auto chg_name_channels_pair : chg_name_channels_map) {
			QTreeWidgetItem *chg_item = new QTreeWidgetItem();
			chg_item->setFlags(
				chg_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
			chg_item->setCheckState(0, Qt::Checked);
			chg_item->setText(0, chg_name_channels_pair.first);
			device_item->addChild(chg_item);

			add_channels(chg_name_channels_pair.second, expanded, chg_item);
			chg_item->setExpanded(expanded);
		}
		device_item->setExpanded(expanded);
	}

	connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
		this, SLOT(update_checks(QTreeWidgetItem*, int)));
}

void SignalTree::add_channels(
	vector<shared_ptr<devices::Channel>> channels, bool expanded,
	QTreeWidgetItem *parent)
{
	for (auto channel : channels) {
		QTreeWidgetItem *ch_item = new QTreeWidgetItem();
		ch_item->setFlags(
			ch_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
		ch_item->setCheckState(0, Qt::Checked);
		ch_item->setText(0, channel->name());
		ch_item->setData(0, Qt::UserRole, QVariant::fromValue(channel));
		parent->addChild(ch_item);

		if (is_show_signals_)
			add_signals(channel->signal_map(), ch_item);
		ch_item->setExpanded(expanded);
	}
}

void SignalTree::add_signals(
	map<devices::Channel::quantity_t, shared_ptr<data::BaseSignal>> signal_map,
	QTreeWidgetItem *parent)
{
	for (auto signal_pair : signal_map) {
		auto signal = signal_pair.second;
		QTreeWidgetItem *signal_item = new QTreeWidgetItem();
		signal_item->setFlags(
			signal_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
		signal_item->setCheckState(0, Qt::Checked);
		signal_item->setText(0, signal->name());
		signal_item->setData(0, Qt::UserRole, QVariant::fromValue(signal));
		parent->addChild(signal_item);
	}
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

void SignalTree::update_checks(QTreeWidgetItem *item, int column)
{
	if(column != 0)
		return;

	disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
		this, SLOT(update_checks(QTreeWidgetItem*, int)));

	update_check_down_recursive(item);
	update_check_up_recursive(item);

	connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
		this, SLOT(update_checks(QTreeWidgetItem*, int)));
}

void SignalTree::update_check_up_recursive(QTreeWidgetItem *item)
{
	if (!item->parent())
		return;

	Qt::CheckState checkState = item->parent()->child(0)->checkState(0);
	for (int i = 1; i < item->parent()->childCount(); ++i) {
		if (item->parent()->child(i)->checkState(0) != checkState) {
			checkState = Qt::PartiallyChecked;
			break;
		}
	}
	item->parent()->setCheckState(0, checkState);

	update_check_up_recursive(item->parent());
}

void SignalTree::update_check_down_recursive(QTreeWidgetItem *item)
{
	Qt::CheckState checkState = item->checkState(0);
	for (int i = 0; i < item->childCount(); ++i) {
		item->child(i)->setCheckState(0, checkState);
		update_check_down_recursive(item->child(i));
	}
}

} // namespace widgets
} // namespace sv

