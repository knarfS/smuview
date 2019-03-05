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

#include <QDebug>
#include <QList>
#include <QModelIndex>
#include <QModelIndexList>
#include <QTreeView>
#include <QVariant>

#include "devicetreeview.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/devices/devicetree/devicetreemodel.hpp"
#include "src/ui/devices/devicetree/treeitem.hpp"

using std::shared_ptr;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace devices {
namespace devicetree {

DeviceTreeView::DeviceTreeView(const Session &session,
		bool is_device_checkable, bool is_channel_group_checkable,
		bool is_channel_checkable, bool is_signal_checkable,
		bool is_configurable_checkable, bool is_config_key_checkable,
		shared_ptr<sv::devices::BaseDevice> expanded_device,
		QWidget *parent) :
	QTreeView(parent),
	session_(session),
	is_device_checkable_(is_device_checkable),
	is_channel_group_checkable_(is_channel_group_checkable),
	is_channel_checkable_(is_channel_checkable),
	is_signal_checkable_(is_signal_checkable),
	is_configurable_checkable_(is_configurable_checkable),
	is_config_key_checkable_(is_config_key_checkable),
	expanded_device_(expanded_device)
{
	setup_ui();
}

void DeviceTreeView::select_channels(
	const vector<shared_ptr<sv::channels::BaseChannel>> channels)
{
	if (!is_channel_checkable_)
		return;

	QList<QStandardItem *> all_items =
		tree_model_->findItems("", Qt::MatchContains | Qt::MatchRecursive);
	for (const auto &item : all_items) {
		if (item->type() == (int)TreeItemType::ChannelItem) {
			auto item_data =
				item->data().value<shared_ptr<sv::channels::BaseChannel>>();
			for (const auto &channel :channels) {
				if (item_data.get() == channel.get())
					item->setCheckState(Qt::Checked);
				else
					item->setCheckState(Qt::Unchecked);
			}
		}
	}
}

vector<shared_ptr<sv::channels::BaseChannel>>
	DeviceTreeView::selected_channels() const
{
	vector<shared_ptr<channels::BaseChannel>> channels;

	if (!is_channel_checkable_)
		return channels;

	QList<QStandardItem *> all_items =
		tree_model_->findItems("", Qt::MatchContains | Qt::MatchRecursive);
	for (const auto &item : all_items) {
		if (item->rowCount() == 0 && item->checkState() > 0 &&
				item->type() == (int)TreeItemType::ChannelItem) {
			channels.push_back(
				item->data().value<shared_ptr<sv::channels::BaseChannel>>());
		}
	}
	return channels;
}

void DeviceTreeView::select_signals(
	const vector<shared_ptr<sv::data::BaseSignal>> signals)
{
	if (!is_signal_checkable_)
		return;

	QList<QStandardItem *> all_items =
		tree_model_->findItems("", Qt::MatchContains | Qt::MatchRecursive);
	for (const auto &item : all_items) {
		if (item->type() == (int)TreeItemType::SignalItem) {
			auto item_data =
				item->data().value<shared_ptr<sv::data::BaseSignal>>();
			for (const auto &signal : signals) {
				if (item_data.get() == signal.get())
					item->setCheckState(Qt::Checked);
				else
					item->setCheckState(Qt::Unchecked);
			}
		}
	}
}

vector<shared_ptr<sv::data::BaseSignal>>
	DeviceTreeView::selected_signals() const
{
	vector<shared_ptr<sv::data::BaseSignal>> signals;

	if (!is_signal_checkable_)
		return signals;

	QList<QStandardItem *> all_items =
		tree_model_->findItems("", Qt::MatchContains | Qt::MatchRecursive);
	for (const auto &item : all_items) {
		if (item->rowCount() == 0 && item->checkState() > 0 &&
				item->type() == (int)TreeItemType::SignalItem) {
			signals.push_back(
				item->data().value<shared_ptr<sv::data::BaseSignal>>());
		}
	}
	return signals;
}

void DeviceTreeView::setup_ui()
{
	tree_model_ = new DeviceTreeModel(session_,
		is_device_checkable_, is_channel_group_checkable_,
		is_channel_checkable_, is_signal_checkable_,
		is_configurable_checkable_, is_config_key_checkable_);

    this->setModel(tree_model_);
	this->setHeaderHidden(true);

	if (expanded_device_) {
		TreeItem *item = tree_model_->find_device(expanded_device_);
		this->expand_recursive(item);
	}
	else {
		this->expandAll();
	}

	connect(tree_model_, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
		this, SLOT(on_rows_inserted(const QModelIndex &, int, int)));
}

void DeviceTreeView::expand_recursive(QStandardItem *item)
{
	if (!item)
		return;

	this->expand(tree_model_->indexFromItem(item));
	for (int i=0; i<item->rowCount(); ++i) {
		expand_recursive(item->child(i));
	}
}

void DeviceTreeView::on_rows_inserted(const QModelIndex &model_index,
	int first, int last)
{
	(void)first;
	(void)last;

	if (!expanded_device_)
		this->expand_recursive(tree_model_->itemFromIndex(model_index));
}

} // namespace devicetree
} // namespace devices
} // namespace ui
} // namespace sv
