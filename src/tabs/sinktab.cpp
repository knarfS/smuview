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
#include <QPushButton>
#include <QHBoxLayout>
#include <QGroupBox>

#include "sinktab.hpp"
#include "src/tabs/devicetab.hpp"
#include "src/data/analog.hpp"
#include "src/data/signalbase.hpp"
#include "src/views/sinkcontrolview.hpp"
#include "src/views/plotview.hpp"
#include "src/views/powerpanelview.hpp"

using std::shared_ptr;

namespace sv {
namespace tabs {

SinkTab::SinkTab(Session &session,
		shared_ptr<devices::HardwareDevice> device, QMainWindow *parent) :
	DeviceTab(session, device, parent)
{
	setup_ui();
}

void SinkTab::setup_ui()
{
	// Device controls
	shared_ptr<views::BaseView> control_view =
		make_shared<views::SinkControlView>(session_, device_, parent_);
	add_view(QString("Device control"), control_view,
		Qt::TopDockWidgetArea, session_);

	// Power panel
	if (device_->voltage_signal() && device_->current_signal()) {
		shared_ptr<views::BaseView> power_panel_view =
			make_shared<views::PowerPanelView>(session_,
				device_->voltage_signal(), device_->current_signal(), parent_);
		add_view(QString("Power Panel"), power_panel_view,
			Qt::TopDockWidgetArea, session_);
	}

	// Current plot
	if (device_->current_signal()) {
		shared_ptr<views::BaseView> plot_view =
			make_shared<views::PlotView>(session_,
				device_->time_data(),
				device_->current_signal()->analog_data(), parent_);
		add_view(QString("Current Graph"), plot_view,
			Qt::BottomDockWidgetArea, session_);
	}
}

} // namespace tabs
} // namespace sv
