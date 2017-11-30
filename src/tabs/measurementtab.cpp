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

#include "src/data/analogdata.hpp"
#include "src/data/basesignal.hpp"
#include "src/views/measurementcontrolview.hpp"
#include "src/views/plotview.hpp"
#include "src/views/valuepanelview.hpp"

namespace sv {
namespace tabs {

MeasurementTab::MeasurementTab(Session &session,
		shared_ptr<devices::HardwareDevice> device, QMainWindow *parent) :
	DeviceTab(session, device, parent)
{
	digits_ = 5;

	setup_ui();
}

void MeasurementTab::setup_ui()
{
	// Device controls
	if (device_->is_controllable()) {
		shared_ptr<views::BaseView> control_view =
			make_shared<views::MeasurementControlView>(session_, device_);
		add_view(tr("Device Control"), control_view,
			Qt::TopDockWidgetArea, session_);
	}

	// Value panel
	if (device_->measurement_signal()) {
		shared_ptr<views::BaseView> value_panel_view =
			make_shared<views::ValuePanelView>(session_,
				device_->measurement_signal());
		add_view(tr("Value Panel"), value_panel_view,
			Qt::TopDockWidgetArea, session_);
	}

	// Value plot
	if (device_->measurement_signal()) {
		shared_ptr<views::BaseView> value_plot_view =
			make_shared<views::PlotView>(session_,
				device_->measurement_signal()->time_data(),
				device_->measurement_signal()->analog_data());
		add_view(tr("Value Graph"), value_plot_view,
			Qt::BottomDockWidgetArea, session_);
	}
}

} // namespace tabs
} // namespace sv

