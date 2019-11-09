/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>
#include <string>

#include <QDebug>
#include <QDockWidget>

#include "uiproxy.hpp"
#include "src/mainwindow.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/python/uihelper.hpp"
#include "src/ui/tabs/basetab.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/plotview.hpp"
#include "src/ui/views/valuepanelview.hpp"

using std::shared_ptr;
using std::string;

namespace sv {
namespace python {

UiProxy::UiProxy(Session &session, shared_ptr<UiHelper> ui_helper) :
	session_(session),
	ui_helper_(ui_helper)
{
	connect(this, &UiProxy::add_device_tab,
		session_.main_window(), &MainWindow::add_device_tab);

	connect(this, &UiProxy::add_data_view,
		ui_helper_.get(), &UiHelper::add_data_view);

	connect(this, &UiProxy::add_control_view,
		ui_helper_.get(), &UiHelper::add_control_view);

	connect(this, QOverload<std::string, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>>::of(&UiProxy::add_plot_view),
		ui_helper_.get(), QOverload<std::string, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>>::of(&UiHelper::add_plot_view));
	connect(this, QOverload<std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>>::of(&UiProxy::add_plot_view),
		ui_helper_.get(), QOverload<std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>>::of(&UiHelper::add_plot_view));
	connect(this, QOverload<std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>, shared_ptr<sv::data::AnalogTimeSignal>>::of(&UiProxy::add_plot_view),
		ui_helper_.get(), QOverload<std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>, shared_ptr<sv::data::AnalogTimeSignal>>::of(&UiHelper::add_plot_view));

	connect(this, &UiProxy::add_power_panel_view,
		ui_helper_.get(), &UiHelper::add_power_panel_view);

	connect(this, QOverload<std::string, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>>::of(&UiProxy::add_value_panel_view),
		ui_helper_.get(), QOverload<std::string, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>>::of(&UiHelper::add_value_panel_view));
	connect(this, QOverload<std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>>::of(&UiProxy::add_value_panel_view),
		ui_helper_.get(), QOverload<std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>>::of(&UiHelper::add_value_panel_view));

	connect(this, &UiProxy::add_signal_to_plot,
		ui_helper_.get(), &UiHelper::add_signal_to_plot);
}


void UiProxy::ui_add_device_tab(shared_ptr<devices::BaseDevice> device)
{
	Q_EMIT add_device_tab(device);
}

string UiProxy::ui_add_data_view(string device_id, Qt::DockWidgetArea area,
	shared_ptr<data::AnalogTimeSignal> signal)
{
	Q_EMIT add_data_view(device_id, area, signal);
	return "data:" + signal->name();
}

string UiProxy::ui_add_control_view(string device_id, Qt::DockWidgetArea area,
	shared_ptr<devices::Configurable> configurable)
{
	Q_EMIT add_control_view(device_id, area, configurable);
	return "control:" + configurable->name();
}

string UiProxy::ui_add_plot_view(string device_id, Qt::DockWidgetArea area,
	shared_ptr<channels::BaseChannel> channel)
{
	Q_EMIT add_plot_view(device_id, area, channel);
	return "plot_ch:" + channel->name();
}

string UiProxy::ui_add_plot_view(string device_id, Qt::DockWidgetArea area,
	shared_ptr<data::AnalogTimeSignal> signal)
{
	Q_EMIT add_plot_view(device_id, area, signal);
	return "plot_sig" + signal->name();
}

string UiProxy::ui_add_plot_view(string device_id, Qt::DockWidgetArea area,
	shared_ptr<data::AnalogTimeSignal> x_signal,
	shared_ptr<data::AnalogTimeSignal> y_signal)
{
	Q_EMIT add_plot_view(device_id, area, x_signal, y_signal);
	return "plot_xy:" + x_signal->name() + ":" + y_signal->name();

}

string UiProxy::ui_add_power_panel_view(string device_id, Qt::DockWidgetArea area,
	shared_ptr<data::AnalogTimeSignal> voltage_signal,
	shared_ptr<data::AnalogTimeSignal> current_signal)
{
	Q_EMIT add_power_panel_view(device_id, area, voltage_signal, current_signal);
	return "powerpanel:" + voltage_signal->name() + ":" + current_signal->name();

}

string UiProxy::ui_add_value_panel_view(string device_id, Qt::DockWidgetArea area,
	shared_ptr<channels::BaseChannel> channel)
{
	Q_EMIT add_value_panel_view(device_id, area, channel);
	return "valuepanel_ch:" + channel->name();
}

string UiProxy::ui_add_value_panel_view(string device_id, Qt::DockWidgetArea area,
	shared_ptr<data::AnalogTimeSignal> signal)
{
	Q_EMIT add_value_panel_view(device_id, area, signal);
	return "valuepanel_sig:" + signal->name();
}

void UiProxy::ui_add_signal_to_plot(string device_id, string view_id,
	shared_ptr<data::AnalogTimeSignal> signal)
{
	Q_EMIT add_signal_to_plot(device_id, view_id, signal);
}

} // namespace python
} // namespace sv

