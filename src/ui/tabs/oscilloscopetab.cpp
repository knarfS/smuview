/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include "oscilloscopetab.hpp"
#include "src/settingsmanager.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/ui/tabs/devicetab.hpp"
#include "src/ui/views/viewhelper.hpp"

namespace sv {
namespace ui {
namespace tabs {

OscilloscopeTab::OscilloscopeTab(Session &session,
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

void OscilloscopeTab::setup_ui()
{
	auto hw_device = static_pointer_cast<sv::devices::HardwareDevice>(device_);

	// Device control(s)
	for (const auto &c_pair : hw_device->configurable_map()) {
		auto configurable = c_pair.second;
		if (!configurable->is_controllable())
			continue;

		/* TODO
		auto view = views::viewhelper::get_view_for_configurable(
			session_, configurable);
		if (view != NULL)
			add_view(view, Qt::TopDockWidgetArea);
		*/
	}

	/*
	for (const auto &chg_pair : device_->channel_group_map()) {
		ui::views::PlotView *plot_view = NULL;
		shared_ptr<data::AnalogScopeSignal> signal;
		for (const auto &channel : chg_pair.second) {
			if (channel->fixed_signal()) {
				auto signal = static_pointer_cast<data::AnalogTimeSignal>(
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
	}
	*/
}

} // namespace tabs
} // namespace ui
} // namespace sv
