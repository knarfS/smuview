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

#include <QHBoxLayout>
#include <QGroupBox>

#include "measurementtab.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/measurementdevice.hpp"
#include "src/views/measurementcontrolview.hpp"
#include "src/views/timeplotview.hpp"
#include "src/views/valuepanelview.hpp"

namespace sv {
namespace tabs {

MeasurementTab::MeasurementTab(Session &session,
		shared_ptr<devices::MeasurementDevice> device, QMainWindow *parent) :
	DeviceTab(session, device, parent),
	measurement_device_(device),
	digits_(5)
{
	setup_ui();
}

void MeasurementTab::setup_ui()
{
	// Device controls
	for (auto c : device_->configurables()) {
		if (c->is_controllable()) {
			shared_ptr<views::BaseView> control_view =
				make_shared<views::MeasurementControlView>(session_, c);
			add_view(control_view, Qt::TopDockWidgetArea, session_);
		}
	}

	// Value panel
	if (measurement_device_->measurement_signal()) {
		shared_ptr<views::BaseView> value_panel_view =
			make_shared<views::ValuePanelView>(session_,
				measurement_device_->measurement_signal());
		add_view(value_panel_view, Qt::TopDockWidgetArea, session_);
	}

	// Value plot
	if (measurement_device_->measurement_signal()) {
		shared_ptr<views::BaseView> value_plot_view =
			make_shared<views::TimePlotView>(
				session_, measurement_device_->measurement_signal());
		add_view(value_plot_view, Qt::BottomDockWidgetArea, session_);
	}
}

} // namespace tabs
} // namespace sv

