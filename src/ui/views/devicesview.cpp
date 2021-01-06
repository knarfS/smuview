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

#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QToolBar>
#include <QUuid>
#include <QVBoxLayout>

#include "devicesview.hpp"
#include "src/devicemanager.hpp"
#include "src/mainwindow.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/devices/userdevice.hpp"
#include "src/ui/devices/devicetree/devicetreemodel.hpp"
#include "src/ui/devices/devicetree/devicetreeview.hpp"
#include "src/ui/devices/devicetree/treeitem.hpp"
#include "src/ui/dialogs/connectdialog.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/baseview.hpp"

using std::shared_ptr;
using sv::ui::devices::devicetree::DeviceTreeModel;
using sv::ui::devices::devicetree::TreeItem;
using sv::ui::devices::devicetree::TreeItemType;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace views {

DevicesView::DevicesView(Session &session, QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	action_add_device_(new QAction(this)),
	action_add_userdevice_(new QAction(this)),
	action_disconnect_device_(new QAction(this))
{
	id_ = "devices:" + util::format_uuid(uuid_);

	setup_ui();
	setup_toolbar();
	connect_signals();
}

QString DevicesView::title() const
{
	return tr("Device Tree");
}

void DevicesView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();
	device_tree_ = new devices::devicetree::DeviceTreeView(session(),
		false, false, false, false, false, false, true, true);
	layout->addWidget(device_tree_);
	layout->setContentsMargins(2, 2, 2, 2);

	this->central_widget_->setLayout(layout);
}

void DevicesView::setup_toolbar()
{
	action_add_device_->setText(tr("Add device"));
	action_add_device_->setIcon(
		QIcon::fromTheme("document-new",
		QIcon(":/icons/document-new.png")));
	connect(action_add_device_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_device_triggered()));

	action_add_userdevice_->setText(tr("Add virtual user device"));
	action_add_userdevice_->setIcon(
		QIcon::fromTheme("tab-new-background",
		QIcon(":/icons/tab-new-background.png")));
	connect(action_add_userdevice_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_userdevice_triggered()));

	action_disconnect_device_->setText(tr("Disconnect device"));
	action_disconnect_device_->setIcon(
		QIcon::fromTheme("edit-delete",
		QIcon(":/icons/edit-delete.png")));
	connect(action_disconnect_device_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_disconnect_device_triggered()));

	toolbar_ = new QToolBar("Device Tree Toolbar");
	toolbar_->addAction(action_add_device_);
	toolbar_->addAction(action_add_userdevice_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_disconnect_device_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void DevicesView::connect_signals()
{
}

void DevicesView::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	(void)settings;
	(void)origin_device;
}

void DevicesView::restore_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	(void)settings;
	(void)origin_device;
}

void DevicesView::on_action_add_device_triggered()
{
	ui::dialogs::ConnectDialog dlg(session().device_manager());
	if (dlg.exec()) {
		auto device = dlg.get_selected_device();
		// NOTE: add_device() must be called, before the device tab
		//       tries to access the device (device is not opend yet).
		session().add_device(device);
		session().main_window()->add_device_tab(device);
	}
}

void DevicesView::on_action_add_userdevice_triggered()
{
	// NOTE: add_user_device() must be called, before the device tab
	//       tries to access the device (device is not opend yet).
	auto device = session().add_user_device();
	session().main_window()->add_device_tab(device);
}

void DevicesView::on_action_disconnect_device_triggered()
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
				arg(signal->display_name()),
			QMessageBox::Yes | QMessageBox::Cancel);

		if (reply == QMessageBox::Yes) {
			signal->clear();
		}
	}
}

} // namespace views
} // namespace ui
} // namespace sv
