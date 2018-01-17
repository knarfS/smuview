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
#include <QHBoxLayout>
#include <QGroupBox>

#include "measurementtab.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/measurementdevice.hpp"
#include "src/views/measurementcontrolview.hpp"
#include "src/views/plotview.hpp"
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
			views::BaseView *control_view =
				new views::MeasurementControlView(session_, c);
			add_view(control_view, Qt::TopDockWidgetArea);
		}
	}

	for (auto ch_name_channel_pair : measurement_device_->channel_name_map()) {
		auto channel = ch_name_channel_pair.second;

		// Value panel(s)
		views::BaseView *value_panel_view =
			new views::ValuePanelView(session_, channel);
		add_view(value_panel_view, Qt::TopDockWidgetArea);

		// Value plot(s)
		views::BaseView *value_plot_view =
			new views::PlotView(session_, channel);
		add_view(value_plot_view, Qt::BottomDockWidgetArea);
	}
}

} // namespace tabs
} // namespace sv

