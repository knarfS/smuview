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

#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QToolBar>
#include <QVBoxLayout>

#include "devicetreeview.hpp"
#include "src/devicemanager.hpp"
#include "src/mainwindow.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/devices/devicetree/devicetreemodel.hpp"
#include "src/ui/devices/devicetree/devicetreeview.hpp"
#include "src/ui/devices/devicetree/treeitem.hpp"
#include "src/ui/dialogs/connectdialog.hpp"
#include "src/ui/views/baseview.hpp"

using std::shared_ptr;
using sv::ui::devices::devicetree::DeviceTreeModel;
using sv::ui::devices::devicetree::TreeItem;
using sv::ui::devices::devicetree::TreeItemType;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace views {

DeviceTreeView::DeviceTreeView(Session &session,
		QWidget *parent) :
	BaseView(session, parent),
	action_add_device_(new QAction(this)),
	action_add_virtualdevice_(new QAction(this)),
	action_disconnect_device_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
	connect_signals();
}

QString DeviceTreeView::title() const
{
	return tr("Device Tree");
}

void DeviceTreeView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();
	device_tree_ = new devices::devicetree::DeviceTreeView(session(),
		false, false, false, false, false, false);
	layout->addWidget(device_tree_);
	layout->setContentsMargins(2, 2, 2, 2);

	this->central_widget_->setLayout(layout);
}

void DeviceTreeView::setup_toolbar()
{
	action_add_device_->setText(tr("Add device"));
	action_add_device_->setIcon(
		QIcon::fromTheme("document-new",
		QIcon(":/icons/document-new.png")));
	connect(action_add_device_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_device_triggered()));

	action_add_virtualdevice_->setText(tr("Add virtual device"));
	action_add_virtualdevice_->setIcon(
		QIcon::fromTheme("tab-new-background",
		QIcon(":/icons/tab-new-background.png")));
	connect(action_add_virtualdevice_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_virtualdevice_triggered()));

	action_disconnect_device_->setText(tr("Disconnect device"));
	action_disconnect_device_->setIcon(
		QIcon::fromTheme("edit-delete",
		QIcon(":/icons/edit-delete.png")));
	connect(action_disconnect_device_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_disconnect_device_triggered()));

	toolbar_ = new QToolBar("Device Tree Toolbar");
	toolbar_->addAction(action_add_device_);
	toolbar_->addAction(action_add_virtualdevice_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_disconnect_device_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void DeviceTreeView::connect_signals()
{
}

void DeviceTreeView::on_action_add_device_triggered()
{
	ui::dialogs::ConnectDialog dlg(session().device_manager());
	if (dlg.exec()) {
		session().main_window()->add_hw_device_tab(dlg.get_selected_device());
	}
}

void DeviceTreeView::on_action_add_virtualdevice_triggered()
{
	session().main_window()->add_virtual_device_tab();
}

void DeviceTreeView::on_action_disconnect_device_triggered()
{
	TreeItem *item = device_tree_->selected_item();
	if (!item)
		return;

	if (item->type() == (int)TreeItemType::DeviceItem) {
		auto device = item->data(DeviceTreeModel::DataRole).
			value<shared_ptr<sv::devices::BaseDevice>>();

		QMessageBox::StandardButton reply = QMessageBox::question(this,
			tr("Close device"),
			tr("Closing the device \"%1\" will also delete all aquired data!").
				arg(device->short_name()),
			QMessageBox::Yes | QMessageBox::Cancel);

		if (reply == QMessageBox::Yes) {
			session().main_window()->remove_tab(device->id());
			session().remove_device(device);
		}
	}
	else if (item->type() == (int)TreeItemType::ChannelItem) {
		/* TODO
		auto channel = item->data(DeviceTreeModel::DataRole).
			value<shared_ptr<sv::channels::BaseChannel>>();

		QMessageBox::StandardButton reply = QMessageBox::question(this,
			tr("Delete signals from channel"),
			tr("Deleting all signals from channel \"%1\" will also delete all aquired data!").
				arg(QString::fromStdString(channel->name())),
			QMessageBox::Yes | QMessageBox::Cancel);

		if (reply == QMessageBox::Yes) {
			channel->clear_signals();
		}
		*/
	}
	else if (item->type() == (int)TreeItemType::SignalItem) {
		auto signal = item->data(DeviceTreeModel::DataRole).
			value<shared_ptr<sv::data::BaseSignal>>();

		QMessageBox::StandardButton reply = QMessageBox::question(this,
			tr("Delete signal"),
			tr("Deleting the signal \"%1\" will also delete all aquired data!").
				arg(signal->name()),
			QMessageBox::Yes | QMessageBox::Cancel);

		if (reply == QMessageBox::Yes) {
			signal->clear();
		}
	}
}

} // namespace views
} // namespace ui
} // namespace sv

