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

#include <unordered_set>

#include <QDebug>
#include <QTreeView>

#include "signaltree.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/device.hpp"
#include "src/devices/hardwaredevice.hpp"

using std::dynamic_pointer_cast;
using std::static_pointer_cast;
using std::unordered_set;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace widgets {

SignalTree::SignalTree(const Session &session,
		bool show_signals, bool multiselect,
		shared_ptr<devices::Device> selected_device,
		QWidget *parent) :
	QTreeWidget(parent),
	session_(session),
	selected_device_(selected_device),
	show_signals_(show_signals),
	multiselect_(multiselect)
{
	setup_ui();
}

vector<shared_ptr<channels::BaseChannel>> SignalTree::selected_channels()
{
	vector<shared_ptr<channels::BaseChannel>> channels;
	auto items = this->checked_items();
	for (auto item : items) {
		QVariant data = item->data(0, Qt::UserRole);
		if (data.isNull())
			continue;

		/*
		shared_ptr<channels::BaseChannel> channel =
			dynamic_pointer_cast<channels::BaseChannel>(data.value);
		*/
		shared_ptr<channels::BaseChannel> channel =
			data.value<shared_ptr<channels::BaseChannel>>();

		if (channel)
			channels.push_back(channel);
	}
	return channels;
}

vector<shared_ptr<data::BaseSignal>> SignalTree::selected_signals()
{
	vector<shared_ptr<data::BaseSignal>> signals;
	for (auto item : this->checked_items()) {
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
	this->setColumnCount(2);
	if (multiselect_)
		this->setSelectionMode(QTreeView::MultiSelection);

	unordered_set<shared_ptr<devices::Device>> devices;
	if (!selected_device_ && &session_)
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
		if (!hw_device)
			continue;

		add_device(hw_device, expanded);
	}

	/*
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
		this, SLOT(update_checks(QTreeWidgetItem*, int)));
	*/
}

void SignalTree::add_device(shared_ptr<devices::HardwareDevice> device,
	bool expanded)
{
	QTreeWidgetItem *device_item = new QTreeWidgetItem();
	device_item->setFlags(
		device_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	device_item->setCheckState(0, Qt::Unchecked);
	device_item->setIcon(0, QIcon(":/icon/smuview.ico"));
	device_item->setText(1, device->full_name());
	this->addTopLevelItem(device_item);

	auto chg_name_channels_map = device->channel_group_name_map();
	for (auto chg_name_channels_pair : chg_name_channels_map) {
		QTreeWidgetItem *chg_item = new QTreeWidgetItem();
		chg_item->setFlags(
			chg_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
		chg_item->setCheckState(0, Qt::Unchecked);
		chg_item->setText(1, chg_name_channels_pair.first);
		device_item->addChild(chg_item);

		for (auto channel : chg_name_channels_pair.second)
			add_channel(channel, expanded, chg_item);

		chg_item->setExpanded(expanded);
	}
	device_item->setExpanded(expanded);

	return;
}

void SignalTree::add_channel(shared_ptr<channels::BaseChannel> channel,
	bool expanded, QTreeWidgetItem *parent)
{
	QTreeWidgetItem *ch_item = new QTreeWidgetItem();
	ch_item->setFlags(
		ch_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	ch_item->setCheckState(0, Qt::Unchecked);
	ch_item->setText(1, channel->name());
	ch_item->setData(0, Qt::UserRole, QVariant::fromValue(channel));
	parent->addChild(ch_item);

	if (show_signals_) {
		for (auto signal_pair : channel->signal_map())
			add_signal(signal_pair.second, ch_item);
	}

	ch_item->setExpanded(expanded);
}

void SignalTree::add_signal(shared_ptr<data::BaseSignal> signal,
	QTreeWidgetItem *parent)
{
	QTreeWidgetItem *signal_item = new QTreeWidgetItem();
	signal_item->setFlags(
		signal_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	signal_item->setCheckState(0, Qt::Unchecked);
	signal_item->setText(1, signal->name());
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

void SignalTree::on_device_added(shared_ptr<devices::HardwareDevice> device)
{
	qWarning() << "SignalTree::on_device_added()" << device->name();
	add_device(device, true);
}

void SignalTree::on_device_removed()
{

}

void SignalTree::on_channel_added()
{

}

void SignalTree::on_channel_removed()
{

}

void SignalTree::on_signal_added()
{

}

void SignalTree::on_signal_removed()
{

}

} // namespace widgets
} // namespace sv

