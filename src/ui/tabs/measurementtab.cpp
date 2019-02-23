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
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/devices/measurementdevice.hpp"
#include "src/ui/views/plotview.hpp"
#include "src/ui/views/valuepanelview.hpp"
#include "src/ui/views/viewhelper.hpp"

namespace sv {
namespace ui {
namespace tabs {

MeasurementTab::MeasurementTab(Session &session,
		shared_ptr<sv::devices::MeasurementDevice> device, QMainWindow *parent) :
	DeviceTab(session, device, parent),
	measurement_device_(device)
{
	setup_ui();
}

void MeasurementTab::setup_ui()
{
	auto hw_device = static_pointer_cast<sv::devices::HardwareDevice>(device_);

	// Device controls
	size_t i = 0;
	for (const auto &c : hw_device->configurables()) {
		if (!c->is_controllable())
			continue;

		auto view = views::viewhelper::get_view_for_configurable(session_, c);
		if (view != NULL) {
			add_view(view, Qt::TopDockWidgetArea);

			// Shown only 2 configurables
			i++;
			if (i >= 2)
				break;
		}
	}

	i = 0;
	for (const auto &chn_ch_pair : measurement_device_->channel_name_map()) {
		auto channel = chn_ch_pair.second;

		// Value panel(s)
		ui::views::BaseView *value_panel_view =
			new ui::views::ValuePanelView(session_, channel);
		add_view(value_panel_view, Qt::TopDockWidgetArea);

		// Value plot(s)
		ui::views::BaseView *value_plot_view =
			new ui::views::PlotView(session_, channel);
		add_view(value_plot_view, Qt::BottomDockWidgetArea);

		// Shown only 2 panles/plots
		i++;
		if (i >= 2)
			break;
	}
}

} // namespace tabs
} // namespace ui
} // namespace sv
