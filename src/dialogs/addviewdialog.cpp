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

#include <QDebug>
#include <QFormLayout>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "addviewdialog.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/channel.hpp"
#include "src/devices/device.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/views/plotview.hpp"
#include "src/views/valuepanelview.hpp"
#include "src/widgets/signaltree.hpp"

using std::static_pointer_cast;

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

	panel_channel_tree_ = new widgets::SignalTree(session_, false, device_);
	form_layout->addWidget(panel_channel_tree_);

	tab_widget_->addTab(panel_widget, title);
}

void AddViewDialog::setup_ui_plot_tab()
{
	QString title("Time Plot");
	QWidget *plot_widget = new QWidget();
	QFormLayout *form_layout = new QFormLayout();
	plot_widget->setLayout(form_layout);

	plot_channel_tree_ = new widgets::SignalTree(session_, true, device_);
	form_layout->addWidget(plot_channel_tree_);

	tab_widget_->addTab(plot_widget, title);
}

vector<shared_ptr<views::BaseView>> AddViewDialog::views()
{
	return views_;
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
		for (auto channel : panel_channel_tree_->selected_channels()) {
			views_.push_back(
				make_shared<views::ValuePanelView>(session_, channel));
		}
		break;
	case 2:
		for (auto channel : plot_channel_tree_->selected_channels()) {
			views_.push_back(make_shared<views::PlotView>(session_, channel));
		}
		for (auto signal : plot_channel_tree_->selected_signals()) {
			//TODO
			auto a_signal = static_pointer_cast<data::AnalogSignal>(signal);
			views_.push_back(make_shared<views::PlotView>(session_, a_signal));
		}

		break;
	default:
		break;
	}

	QDialog::accept();
}

} // namespace dialogs
} // namespace sv
