/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/devices/selectconfigurableform.hpp"
#include "src/ui/devices/signaltree.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/dataview.hpp"
#include "src/ui/views/plotview.hpp"
#include "src/ui/views/valuepanelview.hpp"
#include "src/ui/views/viewhelper.hpp"

using std::static_pointer_cast;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace dialogs {

AddViewDialog::AddViewDialog(const Session &session,
		const shared_ptr<sv::devices::BaseDevice> device,
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
	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(mainIcon);
	this->setWindowTitle(tr("Add View"));
	this->setMinimumWidth(500);

	QVBoxLayout *main_layout = new QVBoxLayout;

	tab_widget_ = new QTabWidget();
	this->setup_ui_control_tab();
	this->setup_ui_panel_tab();
	this->setup_ui_time_plot_tab();
	this->setup_ui_xy_plot_tab();
	this->setup_ui_table_tab();
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
	QString title(tr("Control"));
	QWidget *control_widget = new QWidget();

	configurable_configurable_form_ =
		new ui::devices::SelectConfigurableForm(session_);
	control_widget->setLayout(configurable_configurable_form_);

	tab_widget_->addTab(control_widget, title);
}

void AddViewDialog::setup_ui_panel_tab()
{
	QString title(tr("Panel"));
	QWidget *panel_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	panel_widget->setLayout(layout);

	panel_channel_tree_ = new ui::devices::SignalTree(
		session_, false, true, true, device_);
	layout->addWidget(panel_channel_tree_);

	tab_widget_->addTab(panel_widget, title);
}

void AddViewDialog::setup_ui_time_plot_tab()
{
	QString title(tr("Time Plot"));
	QWidget *plot_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	plot_widget->setLayout(layout);

	time_plot_channel_tree_ = new ui::devices::SignalTree(
		session_, true, true, true, device_);
	layout->addWidget(time_plot_channel_tree_);

	tab_widget_->addTab(plot_widget, title);
}

void AddViewDialog::setup_ui_xy_plot_tab()
{
	QString title(tr("XY Plot"));
	QWidget *plot_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	plot_widget->setLayout(layout);

	xy_plot_x_signal_tree_ = new ui::devices::SignalTree(
		session_, true, true, false, device_);
	layout->addWidget(xy_plot_x_signal_tree_);
	xy_plot_y_signal_tree_ = new ui::devices::SignalTree(
		session_, true, true, false, device_);
	layout->addWidget(xy_plot_y_signal_tree_);

	tab_widget_->addTab(plot_widget, title);
}

void AddViewDialog::setup_ui_table_tab()
{
	QString title(tr("Data Table"));
	QWidget *table_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	table_widget->setLayout(layout);

	table_signal_tree_ = new ui::devices::SignalTree(
		session_, true, true, true, device_);
	layout->addWidget(table_signal_tree_);

	tab_widget_->addTab(table_widget, title);
}

vector<ui::views::BaseView *> AddViewDialog::views()
{
	return views_;
}

void AddViewDialog::accept()
{
	int tab_index = tab_widget_->currentIndex();
	switch (tab_index) {
	case 0: {
			auto configurable =
				configurable_configurable_form_->selected_configurable();
			auto view = views::viewhelper::get_view_for_configurable(
				session_, configurable);

			if (view != NULL)
				views_.push_back(view);
		}
		break;
	case 1:
		for (auto channel : panel_channel_tree_->selected_channels()) {
			views_.push_back(new ui::views::ValuePanelView(session_, channel));
		}
		break;
	case 2:
		for (auto channel : time_plot_channel_tree_->selected_channels()) {
			views_.push_back(new ui::views::PlotView(session_, channel));
		}
		for (auto signal : time_plot_channel_tree_->selected_signals()) {
			auto a_signal = static_pointer_cast<data::AnalogSignal>(signal);
			views_.push_back(new ui::views::PlotView(session_, a_signal));
		}

		break;
	case 3: {
			shared_ptr<data::AnalogSignal> x_signal;
			shared_ptr<data::AnalogSignal> y_signal;
			for (auto signal : xy_plot_x_signal_tree_->selected_signals()) {
				x_signal = static_pointer_cast<data::AnalogSignal>(signal);
				break;
			}
			for (auto signal : xy_plot_y_signal_tree_->selected_signals()) {
				y_signal = static_pointer_cast<data::AnalogSignal>(signal);
				break;
			}
			views_.push_back(new ui::views::PlotView(session_, x_signal, y_signal));
		}
		break;
	case 4:
		for (auto signal : table_signal_tree_->selected_signals()) {
			auto a_signal = static_pointer_cast<data::AnalogSignal>(signal);
			views_.push_back(new ui::views::DataView(session_, a_signal));
		}

		break;
	default:
		break;
	}

	QDialog::accept();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
