/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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
#include <set>

#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "addviewdialog.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/doubleproperty.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/devices/selectconfigurableform.hpp"
#include "src/ui/devices/selectpropertyform.hpp"
#include "src/ui/devices/selectsignalwidget.hpp"
#include "src/ui/devices/devicetree/devicetreeview.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/dataview.hpp"
#include "src/ui/views/powerpanelview.hpp"
#include "src/ui/views/sequenceoutputview.hpp"
#include "src/ui/views/timeplotview.hpp"
#include "src/ui/views/valuepanelview.hpp"
#include "src/ui/views/viewhelper.hpp"
#include "src/ui/views/xyplotview.hpp"

using std::set;
using std::static_pointer_cast;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace dialogs {

AddViewDialog::AddViewDialog(Session &session,
		const shared_ptr<sv::devices::BaseDevice> device,
		int selected_tab,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	device_(device),
	selected_tab_(selected_tab)
{
	setup_ui();
}

void AddViewDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Add View"));
	this->setMinimumWidth(500);

	QVBoxLayout *main_layout = new QVBoxLayout;

	tab_widget_ = new QTabWidget();
	this->setup_ui_control_tab();
	this->setup_ui_sequence_tab();
	this->setup_ui_panel_tab();
	this->setup_ui_time_plot_tab();
	this->setup_ui_xy_plot_tab();
	this->setup_ui_data_table_tab();
	this->setup_ui_power_panel_tab();
	tab_widget_->setCurrentIndex(selected_tab_);
	main_layout->addWidget(tab_widget_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, &QDialogButtonBox::accepted,
		this, &AddViewDialog::accept);
	connect(button_box_, &QDialogButtonBox::rejected,
		this, &AddViewDialog::reject);

	this->setLayout(main_layout);
}

void AddViewDialog::setup_ui_control_tab()
{
	QString title(tr("Control"));
	QWidget *control_widget = new QWidget();

	configurable_configurable_form_ =
		new ui::devices::SelectConfigurableForm(session_);
	configurable_configurable_form_->select_device(device_);
	control_widget->setLayout(configurable_configurable_form_);

	tab_widget_->addTab(control_widget, title);
}

void AddViewDialog::setup_ui_sequence_tab()
{
	QString title(tr("Sequence Output"));
	QWidget *sequence_widget = new QWidget();

	sequence_property_form_ = new ui::devices::SelectPropertyForm(session_);
	sequence_property_form_->select_device(device_);
	sequence_property_form_->filter_config_keys(set<sv::data::DataType>{
		sv::data::DataType::Double});
	sequence_widget->setLayout(sequence_property_form_);

	tab_widget_->addTab(sequence_widget, title);
}

void AddViewDialog::setup_ui_panel_tab()
{
	QString title(tr("Panel"));
	QWidget *panel_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	panel_widget->setLayout(layout);

	panel_channel_tree_ = new ui::devices::devicetree::DeviceTreeView(
		session_, false, false, true, false, false, false, false, false);
	panel_channel_tree_->expand_device(device_);

	layout->addWidget(panel_channel_tree_);

	tab_widget_->addTab(panel_widget, title);
}

void AddViewDialog::setup_ui_time_plot_tab()
{
	QString title(tr("Time Plot"));
	QWidget *plot_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	plot_widget->setLayout(layout);

	time_plot_channel_tree_ = new ui::devices::devicetree::DeviceTreeView(
		session_, false, false, true, true, false, false, false, false);
	time_plot_channel_tree_->expand_device(device_);

	layout->addWidget(time_plot_channel_tree_);

	tab_widget_->addTab(plot_widget, title);
}

void AddViewDialog::setup_ui_xy_plot_tab()
{
	QString title(tr("XY Plot"));
	QWidget *plot_widget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout();
	plot_widget->setLayout(layout);

	QGroupBox *x_signal_group = new QGroupBox(tr("X Signal"));
	QVBoxLayout *x_layout = new QVBoxLayout();
	xy_plot_x_signal_widget_ = new ui::devices::SelectSignalWidget(session_);
	xy_plot_x_signal_widget_->select_device(device_);
	x_layout->addWidget(xy_plot_x_signal_widget_);
	x_signal_group->setLayout(x_layout);
	layout->addWidget(x_signal_group);

	QGroupBox *y_signal_group = new QGroupBox(tr("Y Signal"));
	QVBoxLayout *y_layout = new QVBoxLayout();
	xy_plot_y_signal_widget_ = new ui::devices::SelectSignalWidget(session_);
	xy_plot_y_signal_widget_->select_device(device_);
	y_layout->addWidget(xy_plot_y_signal_widget_);
	y_signal_group->setLayout(y_layout);
	layout->addWidget(y_signal_group);

	tab_widget_->addTab(plot_widget, title);
}

void AddViewDialog::setup_ui_data_table_tab()
{
	QString title(tr("Data Table"));
	QWidget *table_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	table_widget->setLayout(layout);

	data_table_signal_tree_ = new ui::devices::devicetree::DeviceTreeView(
		session_, false, false, false, true, false, false, false, false);
	data_table_signal_tree_->expand_device(device_);

	layout->addWidget(data_table_signal_tree_);

	tab_widget_->addTab(table_widget, title);
}

void AddViewDialog::setup_ui_power_panel_tab()
{
	QString title(tr("Power Panel"));
	QWidget *pp_widget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout();
	pp_widget->setLayout(layout);

	QGroupBox *voltage_signal_group = new QGroupBox(tr("Voltage Signal"));
	QVBoxLayout *voltage_layout = new QVBoxLayout();
	ppanel_voltage_signal_widget_ = new ui::devices::SelectSignalWidget(session_);
	ppanel_voltage_signal_widget_->filter_quantity(data::Quantity::Voltage);
	ppanel_voltage_signal_widget_->select_device(device_);
	voltage_layout->addWidget(ppanel_voltage_signal_widget_);
	voltage_signal_group->setLayout(voltage_layout);
	layout->addWidget(voltage_signal_group);

	QGroupBox *current_signal_group = new QGroupBox(tr("Current Signal"));
	QVBoxLayout *current_layout = new QVBoxLayout();
	ppanel_current_signal_widget_ = new ui::devices::SelectSignalWidget(session_);
	ppanel_current_signal_widget_->filter_quantity(data::Quantity::Current);
	ppanel_current_signal_widget_->select_device(device_);
	current_layout->addWidget(ppanel_current_signal_widget_);
	current_signal_group->setLayout(current_layout);
	layout->addWidget(current_signal_group);

	tab_widget_->addTab(pp_widget, title);
}

vector<ui::views::BaseView *> AddViewDialog::views()
{
	return views_;
}

void AddViewDialog::accept()
{
	int tab_index = tab_widget_->currentIndex();
	switch (tab_index) {
	case 0:
		// Add control view for configurable
		{
			auto configurable =
				configurable_configurable_form_->selected_configurable();
			auto conf_views = views::viewhelper::get_views_for_configurable(
				session_, configurable);

			for (const auto &view : conf_views) {
				views_.push_back(view);
			}
		}
		break;
	case 1:
		// Add sequence view for property
		{
			auto property = sequence_property_form_->selected_property();
			auto *conf_views = new ui::views::SequenceOutputView(session_);
			conf_views->set_property(
				static_pointer_cast<sv::data::properties::DoubleProperty>(property));
			views_.push_back(conf_views);
		}
		break;
	case 2:
		// Add value panel view
		for (const auto &channel : panel_channel_tree_->checked_channels()) {
			auto *conf_views = new ui::views::ValuePanelView(session_);
			conf_views->set_channel(channel);
			views_.push_back(conf_views);
		}
		break;
	case 3:
		// Add time plot view
		for (const auto &channel : time_plot_channel_tree_->checked_channels()) {
			auto *conf_views = new ui::views::TimePlotView(session_);
			conf_views->set_channel(channel);
			views_.push_back(conf_views);
		}
		for (const auto &signal : time_plot_channel_tree_->checked_signals()) {
			auto *conf_views = new ui::views::TimePlotView(session_);
			conf_views->add_signal(static_pointer_cast<data::AnalogTimeSignal>(signal));
			views_.push_back(conf_views);
		}
		break;
	case 4:
		// Add x/y plot view
		{
			auto x_signal = xy_plot_x_signal_widget_->selected_signal();
			auto y_signal = xy_plot_y_signal_widget_->selected_signal();
			if (x_signal != nullptr && y_signal != nullptr) {
				auto *view = new ui::views::XYPlotView(session_);
				view->add_signals(
					static_pointer_cast<data::AnalogTimeSignal>(x_signal),
					static_pointer_cast<data::AnalogTimeSignal>(y_signal));
				views_.push_back(view);
			}
		}
		break;
	case 5:
		// Add data table view
		{
			auto signals = data_table_signal_tree_->checked_signals();
			if (!signals.empty()) {
				auto *view = new ui::views::DataView(session_);
				for (const auto &signal : signals) {
					view->add_signal(
						static_pointer_cast<data::AnalogTimeSignal>(signal));
				}
				views_.push_back(view);
			}
		}
		break;
	case 6:
		// Add power panel view
		{
			auto v_signal = ppanel_voltage_signal_widget_->selected_signal();
			auto c_signal = ppanel_current_signal_widget_->selected_signal();
			if (v_signal != nullptr && c_signal != nullptr) {
				auto *view = new ui::views::PowerPanelView(session_);
				view->set_signals(
					static_pointer_cast<data::AnalogTimeSignal>(v_signal),
					static_pointer_cast<data::AnalogTimeSignal>(c_signal));
				views_.push_back(view);
			}
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
