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

#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QWidget>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "sourcesinktab.hpp"
#include "src/settingsmanager.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/ui/tabs/devicetab.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/powerpanelview.hpp"
#include "src/ui/views/timeplotview.hpp"
#include "src/ui/views/viewhelper.hpp"

namespace sv {
namespace ui {
namespace tabs {

SourceSinkTab::SourceSinkTab(Session &session,
		shared_ptr<sv::devices::HardwareDevice> device, QWidget *parent) :
	DeviceTab(session, device, parent)
{
	if (SettingsManager::restore_settings() &&
			SettingsManager::has_device_settings(device)) {
		restore_settings();
	}
	else
		setup_ui();
}

void SourceSinkTab::setup_ui()
{
	auto hw_device = static_pointer_cast<sv::devices::HardwareDevice>(device_);

	// Device control(s)
	views::BaseView *first_conf_view = nullptr;
	size_t conf_view_count = 0;
	for (const auto &c_pair : hw_device->configurable_map()) {
		auto configurable = c_pair.second;
		if (!configurable->is_controllable())
			continue;

		auto configurable_views = views::viewhelper::get_views_for_configurable(
			session_, configurable);
		for (const auto &configurable_view : configurable_views) {
			conf_view_count++;
			if (!first_conf_view) {
				first_conf_view = configurable_view;
				add_view(configurable_view, Qt::TopDockWidgetArea);
			}
			else
				add_view_ontop(configurable_view, first_conf_view);
		}
	}
	if (first_conf_view != nullptr && conf_view_count > 1) {
		first_conf_view->show();
		first_conf_view->raise();
	}

	// Get signals by their channel group. The signals in a channel are "fixed"
	// for power supplys and loads.
	views::BaseView *first_pp_view = nullptr;
	for (const auto &chg_pair : device_->channel_group_map()) {
		ui::views::TimePlotView *plot_view = nullptr;
		shared_ptr<data::AnalogTimeSignal> voltage_signal;
		shared_ptr<data::AnalogTimeSignal> current_signal;
		for (const auto &channel : chg_pair.second) {
			if (channel->fixed_signal()) {
				auto signal = static_pointer_cast<data::AnalogTimeSignal>(
					channel->actual_signal());

				// Only plot voltage and current
				if (signal->quantity() == data::Quantity::Voltage) {
					voltage_signal = signal;
					// Voltage plot(s)
					if (!plot_view) {
						plot_view = new ui::views::TimePlotView(session_);
						add_view(plot_view, Qt::BottomDockWidgetArea);
					}
					plot_view->add_signal(voltage_signal);
				}
				if (signal->quantity() == data::Quantity::Current) {
					current_signal = signal;
					// Current plot(s)
					if (!plot_view) {
						plot_view = new ui::views::TimePlotView(session_);
						add_view(plot_view, Qt::BottomDockWidgetArea);
					}
					plot_view->add_signal(current_signal);
				}
			}
		}

		if (voltage_signal && current_signal) {
			// PowerPanel(s)
			auto *power_panel_view = new ui::views::PowerPanelView(session_);
			power_panel_view->set_signals(voltage_signal, current_signal);
			if (!first_pp_view) {
				first_pp_view = power_panel_view;
				add_view(power_panel_view, Qt::TopDockWidgetArea);
			}
			else {
				// NOLINTNEXTLINE(readability-suspicious-call-argument)
				add_view_ontop(power_panel_view, first_pp_view);
			}
		}
	}
	if (first_pp_view != nullptr && device_->channel_group_map().size() > 1) {
		first_pp_view->show();
		first_pp_view->raise();
	}
}

} // namespace tabs
} // namespace ui
} // namespace sv
