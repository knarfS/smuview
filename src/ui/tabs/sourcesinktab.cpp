/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "sourcesinktab.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/ui/tabs/devicetab.hpp"
#include "src/ui/views/plotview.hpp"
#include "src/ui/views/powerpanelview.hpp"
#include "src/ui/views/viewhelper.hpp"

namespace sv {
namespace ui {
namespace tabs {

SourceSinkTab::SourceSinkTab(Session &session,
		shared_ptr<sv::devices::HardwareDevice> device, QMainWindow *parent) :
	DeviceTab(session, device, parent)
{
	setup_ui();
}

void SourceSinkTab::setup_ui()
{
	auto hw_device = static_pointer_cast<sv::devices::HardwareDevice>(device_);

	// Device control(s)
	for (const auto &c : hw_device->configurables()) {
		if (!c->is_controllable())
			continue;

		auto view = views::viewhelper::get_view_for_configurable(session_, c);
		if (view != NULL)
			add_view(view, Qt::TopDockWidgetArea);
	}

	// Get signals by their channel group. The signals in a channel are "fixed"
	// for power supplys and loads.
	for (const auto &chg_name_signals_pair : device_->channel_group_name_map()) {
		ui::views::PlotView *plot_view = NULL;
		shared_ptr<data::AnalogSignal> voltage_signal;
		shared_ptr<data::AnalogSignal> current_signal;
		auto channels = chg_name_signals_pair.second;
		for (const auto &channel : channels) {
			if (channel->has_fixed_signal()) {
				auto signal = static_pointer_cast<data::AnalogSignal>(
					channel->actual_signal());

				// Only plot voltage and current
				if (signal->quantity() == data::Quantity::Voltage) {
					voltage_signal = signal;
					// Voltage plot(s)
					if (!plot_view) {
						plot_view = new ui::views::PlotView(session_, voltage_signal);
						add_view(plot_view, Qt::BottomDockWidgetArea);
					}
					else
						plot_view->add_time_curve(voltage_signal);

				}
				if (signal->quantity() == data::Quantity::Current) {
					current_signal = signal;
					// Current plot(s)
					if (!plot_view) {
						plot_view = new ui::views::PlotView(session_, current_signal);
						add_view(plot_view, Qt::BottomDockWidgetArea);
					}
					else
						plot_view->add_time_curve(current_signal);
				}
			}
		}

		if (voltage_signal && current_signal) {
			// PowerPanel(s)
			ui::views::BaseView *power_panel_view = new ui::views::PowerPanelView(
				session_, voltage_signal, current_signal);
			add_view(power_panel_view, Qt::TopDockWidgetArea);
		}
	}
}

} // namespace tabs
} // namespace ui
} // namespace sv
