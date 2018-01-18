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

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "sourcesinktab.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/device.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/tabs/devicetab.hpp"
#include "src/views/sinkcontrolview.hpp"
#include "src/views/sourcecontrolview.hpp"
#include "src/views/plotview.hpp"
#include "src/views/powerpanelview.hpp"

namespace sv {
namespace tabs {

SourceSinkTab::SourceSinkTab(Session &session,
		shared_ptr<devices::HardwareDevice> device, QMainWindow *parent) :
	DeviceTab(session, device, parent)
{
	setup_ui();
}

void SourceSinkTab::setup_ui()
{
	// Device control(s)
	for (auto c : device_->configurables()) {
		if (c->is_controllable()) {
			if (device_->type() == devices::DeviceType::PowerSupply)
				add_view(new views::SourceControlView(session_, c),
					Qt::TopDockWidgetArea);
			else if (device_->type() == devices::DeviceType::ElectronicLoad)
				add_view(new views::SinkControlView(session_, c),
					Qt::TopDockWidgetArea);
		}
	}

	// Get signals by their channel group. The signals in a channel are "fixed"
	// for power supplys and loads.
	for (auto chg_name_signals_pair : device_->channel_group_name_map()) {
		shared_ptr<data::AnalogSignal> voltage_signal;
		shared_ptr<data::AnalogSignal> current_signal;
		auto channels = chg_name_signals_pair.second;
		for (auto channel : channels) {
			if (channel->has_fixed_signal()) {
				auto signal = static_pointer_cast<data::AnalogSignal>(
					channel->actual_signal());

				// Only plot voltage and current

				if (signal->sr_quantity() == sigrok::Quantity::VOLTAGE) {
					voltage_signal = signal;
					// Voltage plot(s)
					add_view(new views::PlotView(session_, voltage_signal),
						Qt::BottomDockWidgetArea);
				}
				if (signal->sr_quantity() == sigrok::Quantity::CURRENT) {
					current_signal = signal;
					// Voltage plot(s)
					add_view(new views::PlotView(session_, current_signal),
						Qt::BottomDockWidgetArea);
				}
			}
			else {
				// Channel plot(s)
				// TODO: Check for channels without data (Digi 35 CPU "CH1")
				//add_view(new views::PlotView(session_, channel),
				//	Qt::BottomDockWidgetArea);
			}
		}

		if (voltage_signal && current_signal) {
			// PowerPanel(s)
			views::BaseView *power_panel_view = new views::PowerPanelView(
				session_, voltage_signal, current_signal);
			add_view(power_panel_view, Qt::TopDockWidgetArea);

			// UI Plots
			views::BaseView *ui_plot_view = new views::PlotView(
				session_, voltage_signal, current_signal);
			add_view(ui_plot_view, Qt::BottomDockWidgetArea);
		}
	}
}

} // namespace tabs
} // namespace sv
