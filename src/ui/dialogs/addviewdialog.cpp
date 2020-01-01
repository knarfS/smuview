/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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
#include "src/ui/views/plotview.hpp"
#include "src/ui/views/sequenceoutputview.hpp"
#include "src/ui/views/valuepanelview.hpp"
#include "src/ui/views/viewhelper.hpp"

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
	this->setup_ui_panel_tab();
	this->setup_ui_time_plot_tab();
	this->setup_ui_xy_plot_tab();
	this->setup_ui_table_tab();
	this->setup_ui_sequence_tab();
	tab_widget_->setCurrentIndex(selected_tab_);
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
	configurable_configurable_form_->select_device(device_);
	control_widget->setLayout(configurable_configurable_form_);

	tab_widget_->addTab(control_widget, title);
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

void AddViewDialog::setup_ui_table_tab()
{
	QString title(tr("Data Table"));
	QWidget *table_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	table_widget->setLayout(layout);

	table_signal_tree_ = new ui::devices::devicetree::DeviceTreeView(
		session_, false, false, false, true, false, false, false, false);
	table_signal_tree_->expand_device(device_);

	layout->addWidget(table_signal_tree_);

	tab_widget_->addTab(table_widget, title);
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
			auto view = views::viewhelper::get_view_for_configurable(
				session_, configurable);

			if (view != nullptr)
				views_.push_back(view);
		}
		break;
	case 1:
		// Add value panel view
		for (const auto &channel : panel_channel_tree_->checked_channels()) {
			views_.push_back(new ui::views::ValuePanelView(session_, channel));
		}
		break;
	case 2:
		// Add plot view
		for (const auto &channel : time_plot_channel_tree_->checked_channels()) {
			views_.push_back(new ui::views::PlotView(session_, channel));
		}
		for (const auto &signal : time_plot_channel_tree_->checked_signals()) {
			auto a_signal = static_pointer_cast<data::AnalogTimeSignal>(signal);
			views_.push_back(new ui::views::PlotView(session_, a_signal));
		}
		break;
	case 3:
		// Add x/y plot view
		{
			auto x_signal = xy_plot_x_signal_widget_->selected_signal();
			auto y_signal = xy_plot_y_signal_widget_->selected_signal();
			if (x_signal != nullptr && y_signal != nullptr) {
				views_.push_back(new ui::views::PlotView(session_,
					static_pointer_cast<data::AnalogTimeSignal>(x_signal),
					static_pointer_cast<data::AnalogTimeSignal>(y_signal)));
			}
		}
		break;
	case 4:
		// Add data table view
		{
			auto signals = table_signal_tree_->checked_signals();
			if (signals.size() > 0) {
				auto view = new ui::views::DataView(session_,
					static_pointer_cast<data::AnalogTimeSignal>(signals[0]));
				for (size_t i=1; i<signals.size(); ++i) {
					view->add_signal(
						static_pointer_cast<data::AnalogTimeSignal>(signals[i]));
				}
				views_.push_back(view);
			}
		}
		break;
	case 5:
		// Add sequence view for property
		{
			auto property = sequence_property_form_->selected_property();
			auto view = new ui::views::SequenceOutputView(session_,
				static_pointer_cast<sv::data::properties::DoubleProperty>(property));
			if (view != nullptr)
				views_.push_back(view);
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
