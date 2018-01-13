/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QFormLayout>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "addviewdialog.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/channel.hpp"
#include "src/devices/device.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/views/timeplotview.hpp"
#include "src/views/valuepanelview.hpp"

using std::unordered_set;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace dialogs {

AddViewDialog::AddViewDialog(const Session &session,
		const shared_ptr<devices::HardwareDevice> device,
		int selected_view_type,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	device_(device),
	selected_view_type_(selected_view_type)
{
	setup_ui();
}

void AddViewDialog::setup_ui()
{
	this->setWindowTitle(tr("Add View"));

	QVBoxLayout *main_layout = new QVBoxLayout;

	tab_widget_ = new QTabWidget();
	this->setup_ui_control_tab();
	this->setup_ui_panel_tab();
	this->setup_ui_plot_tab();
	tab_widget_->setCurrentIndex(selected_view_type_);
	main_layout->addWidget(tab_widget_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void AddViewDialog::setup_ui_control_tab()
{
	QString title("Control");
	QWidget *control_widget = new QWidget();
	QFormLayout *form_layout = new QFormLayout();
	control_widget->setLayout(form_layout);

	tab_widget_->addTab(control_widget, title);
}

void AddViewDialog::setup_ui_panel_tab()
{
	QString title("Panel");
	QWidget *panel_widget = new QWidget();
	QFormLayout *form_layout = new QFormLayout();
	panel_widget->setLayout(form_layout);

	panel_channel_tree_ = setup_ui_channel_tree();
	form_layout->addWidget(panel_channel_tree_);

	tab_widget_->addTab(panel_widget, title);
}

void AddViewDialog::setup_ui_plot_tab()
{
	QString title("Time Plot");
	QWidget *plot_widget = new QWidget();
	QFormLayout *form_layout = new QFormLayout();
	plot_widget->setLayout(form_layout);

	plot_channel_tree_ = setup_ui_channel_tree();
	form_layout->addWidget(plot_channel_tree_);

	tab_widget_->addTab(plot_widget, title);
}

QTreeWidget * AddViewDialog::setup_ui_channel_tree()
{
	// TODO hierarchy checkboxes
	QTreeWidget *channel_tree = new QTreeWidget();
	channel_tree->setColumnCount(1);
	channel_tree->setSelectionMode(QTreeView::MultiSelection);

	unordered_set<shared_ptr<devices::Device>> devices;
	if (!device_)
		devices = session_.devices();
	else
		devices.insert(device_);

	for (auto device : devices) {
		// Tree root
		QTreeWidgetItem *device_item = new QTreeWidgetItem(channel_tree);
		device_item->setFlags(device_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
		device_item->setCheckState(0, Qt::Checked);
		device_item->setIcon(0, QIcon(":/icon/smuview.ico"));
		device_item->setText(0, device->full_name());
		device_item->setExpanded(true);

		auto chg_name_channels_map = device_->channel_group_name_map();
		for (auto chg_name_channels_pair : chg_name_channels_map) {
			QTreeWidgetItem *chg_item = new QTreeWidgetItem();
			chg_item->setFlags(chg_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
			chg_item->setCheckState(0, Qt::Checked);
			chg_item->setText(0, chg_name_channels_pair.first);
			chg_item->setExpanded(true);

			auto channels = chg_name_channels_pair.second;
			for (auto channel : channels) {
				QTreeWidgetItem *ch_item = new QTreeWidgetItem();
				ch_item->setFlags(ch_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
				ch_item->setCheckState(0, Qt::Checked);
				ch_item->setText(0, channel->name());
				ch_item->setData(0, Qt::UserRole, QVariant::fromValue(channel));

				chg_item->addChild(ch_item);
			}

			device_item->addChild(chg_item);
		}
	}

	connect(channel_tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
		this, SLOT(update_checks(QTreeWidgetItem*, int)));

	return channel_tree;
}

shared_ptr<views::BaseView> AddViewDialog::view()
{
	return view_;
}

void AddViewDialog::accept()
{
	shared_ptr<devices::Channel> channel;
	int tab_index = tab_widget_->currentIndex();
	switch (tab_index) {
	case 0:
		//view_ = nullptr;
		break;
	case 1:
		for (auto item : panel_channel_tree_->selectedItems()) {
			// TODO: static_pointer_cast<devices::Channel> ?
			channel = item->data(0, Qt::UserRole).
				value<shared_ptr<devices::Channel>>();
			break; // SingleSelect
		}
		view_ = make_shared<views::ValuePanelView>(session_, channel);
		break;
	case 2:
		for (auto item : plot_channel_tree_->selectedItems()) {
			// TODO: static_pointer_cast<devices::Channel> ?
			channel = item->data(0, Qt::UserRole).
				value<shared_ptr<devices::Channel>>();
			break; // SingleSelect
		}
		//view_ = make_shared<views::TimePlotView>(session_, channel);
		break;
	default:
		break;
	}

	QDialog::accept();
}

void AddViewDialog::update_checks(QTreeWidgetItem *item, int column)
{
	if(column != 0)
		return;

	recursive_down_checks(item);
	recursive_up_checks(item);
}

void AddViewDialog::recursive_up_checks(QTreeWidgetItem *item)
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

	//if (parent->parent())
	//	recursive_up_checks(parent->parent());
}

void AddViewDialog::recursive_down_checks(QTreeWidgetItem *item)
{
	Qt::CheckState checkState = item->checkState(0);
	for (int i = 0; i < item->childCount(); ++i) {
		item->child(i)->setCheckState(0, checkState);
		recursive_down_checks(item->child(i));
	}
}

} // namespace dialogs
} // namespace sv
