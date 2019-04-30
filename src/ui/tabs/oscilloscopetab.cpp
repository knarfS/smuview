/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/oscilloscopedevice.hpp"
#include "src/ui/tabs/devicetab.hpp"
#include "src/ui/views/scopeplotview.hpp"
#include "src/ui/views/scopetriggercontrolview.hpp"
#include "src/ui/views/viewhelper.hpp"

namespace sv {
namespace ui {
namespace tabs {

OscilloscopeTab::OscilloscopeTab(Session &session,
		shared_ptr<sv::devices::HardwareDevice> device, QWidget *parent) :
	DeviceTab(session, device, parent)
{
	setup_ui();
}

void OscilloscopeTab::setup_ui()
{
	auto scope_device = static_pointer_cast<sv::devices::OscilloscopeDevice>(device_);

	// Device control(s)
	for (const auto &c_pair : scope_device->configurable_map()) {
		auto configurable = c_pair.second;
		if (!configurable->is_controllable())
			continue;

		auto view = views::viewhelper::get_view_for_configurable(
			session_, configurable);
		if (view != NULL)
			add_view(view, Qt::BottomDockWidgetArea);

		// TODO: Use ViewHelper also for trigger view
		if (configurable->name().empty()) {
			add_view(
				new views::ScopeTriggerControlView(session_, configurable),
				Qt::BottomDockWidgetArea);
		}
	}

	size_t added_channels = 0;
	ui::views::ScopePlotView *plot_view = NULL;
	for (const auto &chg_pair : device_->channel_group_map()) {
		/* TODO: for now, only the first two channles are displayed. */
		if (added_channels >= 2)
			break;

		/* TODO: We assume, that every channel group has just one channel. */
		// Only get the first channel, and ignore the others
		auto channel = chg_pair.second.at(0);
		if (!channel)
			continue;

		++added_channels;

		// Voltage plot
		if (!plot_view) {
			plot_view = new ui::views::ScopePlotView(session_, scope_device, channel);
			add_view(plot_view, Qt::TopDockWidgetArea);
		}
		//else TODO
		//	plot_view->add_channel(channel);
	}
}

} // namespace tabs
} // namespace ui
} // namespace sv
