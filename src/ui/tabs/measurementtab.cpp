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

#include <QDebug>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QWidget>

#include "measurementtab.hpp"
#include "src/util.hpp"
#include "src/settingsmanager.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/devices/measurementdevice.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/timeplotview.hpp"
#include "src/ui/views/valuepanelview.hpp"
#include "src/ui/views/viewhelper.hpp"

namespace sv {
namespace ui {
namespace tabs {

MeasurementTab::MeasurementTab(Session &session,
		shared_ptr<sv::devices::MeasurementDevice> device, QWidget *parent) :
	DeviceTab(session, device, parent),
	measurement_device_(device)
{
	if (SettingsManager::has_device_settings(device))
		restore_settings();
	else
		setup_ui();
}

void MeasurementTab::setup_ui()
{
	auto hw_device = static_pointer_cast<sv::devices::HardwareDevice>(device_);

	// Device controls
	views::BaseView *first_conf_view = nullptr;
	for (const auto &c_pair : hw_device->configurable_map()) {
		// Ignore logic controls from the demo devive.
		if (c_pair.first == "Logic")
			continue;

		auto configurable = c_pair.second;
		if (!configurable->is_controllable())
			continue;

		auto configurable_view = views::viewhelper::get_view_for_configurable(
			session_, configurable);
		if (configurable_view) {
			if (!first_conf_view) {
				first_conf_view = configurable_view;
				add_view(configurable_view, Qt::TopDockWidgetArea);
			}
			else
				add_view_ontop(configurable_view, first_conf_view);
		}
	}
	if (first_conf_view != nullptr &&
			hw_device->configurable_map().size() > 1) {
		first_conf_view->show();
		first_conf_view->raise();
	}

	views::BaseView *first_panel_view = nullptr;
	for (const auto &ch_pair : measurement_device_->channel_map()) {
		// Ignore digital channels (starting with "D") from the demo devive.
		if (util::starts_with(ch_pair.first, "D"))
			continue;

		auto channel = ch_pair.second;

		// Value panel(s)
		auto value_panel_view = new ui::views::ValuePanelView(session_);
		value_panel_view->set_channel(channel);
		if (!first_panel_view) {
			first_panel_view = value_panel_view;
			add_view(value_panel_view, Qt::TopDockWidgetArea);
		}
		else
			add_view_ontop(value_panel_view, first_panel_view);

		// Value plot(s)
		auto value_plot_view = new ui::views::TimePlotView(session_);
		value_plot_view->set_channel(channel);
		add_view(value_plot_view, Qt::BottomDockWidgetArea);
	}
	if (first_panel_view != nullptr &&
			measurement_device_->channel_map().size() > 1) {
		first_panel_view->show();
		first_panel_view->raise();
	}
}

} // namespace tabs
} // namespace ui
} // namespace sv
