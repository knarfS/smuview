/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QPushButton>
#include <QHBoxLayout>
#include <QGroupBox>

#include "sinktab.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/sourcesinkdevice.hpp"
#include "src/views/sinkcontrolview.hpp"
#include "src/views/powerpanelview.hpp"
#include "src/views/timeplotview.hpp"

using std::shared_ptr;

namespace sv {
namespace tabs {

SinkTab::SinkTab(Session &session,
		shared_ptr<devices::SourceSinkDevice> device, QMainWindow *parent) :
	DeviceTab(session, device, parent),
	sourcesink_device_(device)
{
	setup_ui();
}

void SinkTab::setup_ui()
{
	// Device controls
	for (auto c : device_->configurables()) {
		if (c->is_controllable()) {
			shared_ptr<views::BaseView> control_view =
				make_shared<views::SinkControlView>(session_, c);
			add_view(control_view, Qt::TopDockWidgetArea, session_);
		}
	}

	// Power panel
	if (sourcesink_device_->voltage_signal() &&
			sourcesink_device_->current_signal()) {
		shared_ptr<views::BaseView> power_panel_view =
			make_shared<views::PowerPanelView>(session_,
				sourcesink_device_->voltage_signal(),
				sourcesink_device_->current_signal());
		add_view(power_panel_view, Qt::TopDockWidgetArea, session_);
	}

	// Voltage plot
	if (sourcesink_device_->voltage_signal()) {
		shared_ptr<views::BaseView> voltage_plot_view =
			make_shared<views::TimePlotView>(
				session_, sourcesink_device_->voltage_signal());
		add_view(voltage_plot_view, Qt::BottomDockWidgetArea, session_);
	}

	// Current plot
	if (sourcesink_device_->current_signal()) {
		shared_ptr<views::BaseView> current_plot_view =
			make_shared<views::TimePlotView>(
				session_, sourcesink_device_->current_signal());
		add_view(current_plot_view, Qt::BottomDockWidgetArea, session_);
	}

	// UI plot
	/*
	if (device_->current_signal()) {
		shared_ptr<views::BaseView> ui_plot_view =
			make_shared<views::PlotView>(session_,
				device_->voltage_signal()->analog_data(),
				device_->current_signal()->analog_data(), parent_);
		add_view(ui_plot_view, Qt::BottomDockWidgetArea, session_);
	}
	*/
}

} // namespace tabs
} // namespace sv
