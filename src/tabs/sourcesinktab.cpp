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

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "sourcesinktab.hpp"
#include "src/tabs/devicetab.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/views/sinkcontrolview.hpp"
#include "src/views/sourcecontrolview.hpp"
#include "src/views/powerpanelview.hpp"
#include "src/views/timeplotview.hpp"

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
			shared_ptr<views::BaseView> control_view;
			if (device_->type() == devices::HardwareDevice::Type::POWER_SUPPLY)
				control_view = make_shared<views::SourceControlView>(session_, c);
			else if (device_->type() == devices::HardwareDevice::Type::ELECTRONIC_LOAD)
				control_view = make_shared<views::SinkControlView>(session_, c);

			add_view(control_view, Qt::TopDockWidgetArea, session_);
		}
	}

	const auto cg_name_quantity_signal_map =
		device_->cg_name_sr_quantity_signals_map();

	for (auto cg_name_map_pair : cg_name_quantity_signal_map) {
		auto quantity_signal_map = cg_name_map_pair.second;

		// Power panel(s)
		if (quantity_signal_map.count(sigrok::Quantity::VOLTAGE) != 0 &&
				quantity_signal_map.count(sigrok::Quantity::CURRENT) != 0) {
			shared_ptr<views::BaseView> power_panel_view =
				make_shared<views::PowerPanelView>(session_,
					quantity_signal_map[sigrok::Quantity::VOLTAGE],
					quantity_signal_map[sigrok::Quantity::CURRENT]);
			add_view(power_panel_view, Qt::TopDockWidgetArea, session_);
		}

		// Voltage plot(s)
		if (quantity_signal_map.count(sigrok::Quantity::VOLTAGE) != 0) {
			shared_ptr<views::BaseView> voltage_plot_view =
				make_shared<views::TimePlotView>(
					session_, quantity_signal_map[sigrok::Quantity::VOLTAGE]);
			add_view(voltage_plot_view, Qt::BottomDockWidgetArea, session_);
		}

		// Current plot(s)
		if (quantity_signal_map.count(sigrok::Quantity::CURRENT) != 0) {
			shared_ptr<views::BaseView> current_plot_view =
				make_shared<views::TimePlotView>(
					session_, quantity_signal_map[sigrok::Quantity::CURRENT]);
			add_view(current_plot_view, Qt::BottomDockWidgetArea, session_);
		}
	}

	// UI plot(s)
	/*
	if (device_->current_signal()) {
		shared_ptr<views::BaseView> ui_plot_view =
			make_shared<views::PlotView>(session_,
				device_->voltage_signal()->analog_data(),
				device_->current_signal()->analog_data());
		add_view(ui_plot_view, Qt::BottomDockWidgetArea, session_);
	}
	*/
}

} // namespace tabs
} // namespace sv
