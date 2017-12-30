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
#include "src/data/basesignal.hpp"
#include "src/data/analogdata.hpp"
#include "src/devices/device.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/views/plotview.hpp"
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

	panel_signal_tree_ = setup_ui_signal_tree();
	form_layout->addWidget(panel_signal_tree_);

	tab_widget_->addTab(panel_widget, title);
}

void AddViewDialog::setup_ui_plot_tab()
{
	QString title("Plot");
	QWidget *plot_widget = new QWidget();
	QFormLayout *form_layout = new QFormLayout();
	plot_widget->setLayout(form_layout);

	plot_signal_tree_ = setup_ui_signal_tree();
	form_layout->addWidget(plot_signal_tree_);

	tab_widget_->addTab(plot_widget, title);
}

QTreeWidget * AddViewDialog::setup_ui_signal_tree()
{
	// TODO hierarchy checkboxes
	QTreeWidget *signal_tree = new QTreeWidget();
	signal_tree->setColumnCount(2);
	//signal_tree->setSelectionMode(QTreeView::MultiSelection);

	unordered_set<shared_ptr<devices::Device>> devices;
	if (!device_)
		devices = session_.devices();
	else
		devices.insert(device_);

	for (auto device : devices) {
		// Tree root

		// QTreeWidgetItem(QTreeWidget * parent, int type = Type)
		QTreeWidgetItem *device_item = new QTreeWidgetItem(signal_tree);
		device_item->setText(0, device->full_name());
		device_item->setText(1, "");

		if (device_ && device.get() == device_.get())
			device_item->setExpanded(true);

		map<string, shared_ptr<sigrok::ChannelGroup>> sr_cgs = device_->sr_device()->channel_groups();
		map<string, shared_ptr<sigrok::ChannelGroup>>::iterator it = sr_cgs.begin();
		for (; it != sr_cgs.end(); ++it) {
			QTreeWidgetItem *cg_item = new QTreeWidgetItem();
			cg_item->setText(0, QString::fromStdString(it->first));
			cg_item->setText(1, QString::fromStdString(it->first));

			//QVariant var = QVariant::fromValue(signal);
			//signal_item->setData(0, Qt::UserRole, var);

			device_item->addChild(cg_item);
		}

		auto hw_device = static_pointer_cast<devices::HardwareDevice>(device);
		for (shared_ptr<data::BaseSignal> signal : hw_device->all_signals()) {
			QTreeWidgetItem *signal_item = new QTreeWidgetItem();
			signal_item->setText(0, signal->name());
			signal_item->setText(1, signal->internal_name());

			QVariant var = QVariant::fromValue(signal);
			signal_item->setData(0, Qt::UserRole, var);

			device_item->addChild(signal_item);
		}
	}

	return signal_tree;
}

shared_ptr<views::BaseView> AddViewDialog::view()
{
	return view_;
}

void AddViewDialog::accept()
{
	shared_ptr<data::BaseSignal> signal;
	int tab_index = tab_widget_->currentIndex();
	switch (tab_index) {
	case 0:
		//view_ = nullptr;
		break;
	case 1:
		for (auto item : panel_signal_tree_->selectedItems()) {
			signal = item->data(0, Qt::UserRole).
				value<shared_ptr<data::BaseSignal>>();
			break; // SingleSelect
		}
		view_ = make_shared<views::ValuePanelView>(session_, signal);
		break;
	case 2:
		for (auto item : plot_signal_tree_->selectedItems()) {
			signal = item->data(0, Qt::UserRole).
				value<shared_ptr<data::BaseSignal>>();
			break; // SingleSelect
		}
		view_ = make_shared<views::PlotView>(session_,
			signal->time_data(), signal->analog_data());
		break;
	default:
		break;
	}

	QDialog::accept();
}

} // namespace dialogs
} // namespace sv
