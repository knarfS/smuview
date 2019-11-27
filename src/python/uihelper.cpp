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

#include "uihelper.hpp"
#include "src/mainwindow.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/devices/configurable.hpp"
#include "src/ui/tabs/basetab.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/dataview.hpp"
#include "src/ui/views/plotview.hpp"
#include "src/ui/views/powerpanelview.hpp"
#include "src/ui/views/valuepanelview.hpp"
#include "src/ui/views/viewhelper.hpp"

using std::shared_ptr;
using std::string;

namespace sv {
namespace python {

UiHelper::UiHelper(Session &session) :
	session_(session)
{
}

void UiHelper::add_data_view(std::string device_id, Qt::DockWidgetArea area,
	shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	tab->add_view(new ui::views::DataView(session_, signal), area);
}

void UiHelper::add_control_view(std::string device_id, Qt::DockWidgetArea area,
	shared_ptr<sv::devices::Configurable> configurable)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	tab->add_view(
		ui::views::viewhelper::get_view_for_configurable(session_, configurable),
		area);
}

void UiHelper::add_plot_view(std::string device_id, Qt::DockWidgetArea area,
	shared_ptr<sv::channels::BaseChannel> channel)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	tab->add_view(new ui::views::PlotView(session_, channel), area);
}

void UiHelper::add_plot_view(std::string device_id, Qt::DockWidgetArea area,
	shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	tab->add_view(new ui::views::PlotView(session_, signal), area);
}

void UiHelper::add_plot_view(std::string device_id, Qt::DockWidgetArea area,
	shared_ptr<sv::data::AnalogTimeSignal> x_signal,
	shared_ptr<sv::data::AnalogTimeSignal> y_signal)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	tab->add_view(new ui::views::PlotView(session_, x_signal, y_signal), area);
}

void UiHelper::add_power_panel_view(std::string device_id,
	Qt::DockWidgetArea area,
	shared_ptr<sv::data::AnalogTimeSignal> voltage_signal,
	shared_ptr<sv::data::AnalogTimeSignal> current_signal)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	tab->add_view(
		new ui::views::PowerPanelView(session_, voltage_signal, current_signal),
		area);
}

void UiHelper::add_value_panel_view(std::string device_id,
	Qt::DockWidgetArea area, shared_ptr<sv::channels::BaseChannel> channel)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	tab->add_view(new ui::views::ValuePanelView(session_, channel), area);
}

void UiHelper::add_value_panel_view(std::string device_id,
	Qt::DockWidgetArea area, shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	tab->add_view(new ui::views::ValuePanelView(session_, signal), area);
}

void UiHelper::add_signal_to_data_view(std::string device_id,
	std::string view_id, shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	auto view = tab->get_view_from_view_id(view_id);
	if (!view)
		return;

	((ui::views::DataView *)view)->add_signal(signal);
}

void UiHelper::add_signal_to_plot_view(std::string device_id, std::string view_id,
	shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab) {
		qWarning() << "UiHelper::add_signal_to_plot_view(): Tab not found: " <<
			QString::fromStdString(device_id);
		return;
	}
	auto view = tab->get_view_from_view_id(view_id);
	if (!view) {
		qWarning() << "UiHelper::add_signal_to_plot_view(): View not found: " <<
			QString::fromStdString(view_id);
		return;
	}
	auto plot_view = dynamic_cast<ui::views::PlotView *>(view);
	if (!plot_view) {
		qWarning() << "UiHelper::add_signal_to_plot_view(): View is not a " <<
			"plot view: " << QString::fromStdString(view_id);
		return;
	}

	if (plot_view->id().rfind("plot_xy", 0) == 0)
		((ui::views::PlotView *)plot_view)->add_xy_curve(signal);
	else
		((ui::views::PlotView *)plot_view)->add_time_curve(signal);
}

void UiHelper::add_signals_to_xy_plot_view(std::string device_id,
	std::string view_id,
	shared_ptr<sv::data::AnalogTimeSignal> x_signal,
	shared_ptr<sv::data::AnalogTimeSignal> y_signal)
{
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab) {
		qWarning() << "UiHelper::add_signals_to_xy_plot_view(): Tab not " <<
			"found:" << QString::fromStdString(device_id);
		return;
	}
	auto view = tab->get_view_from_view_id(view_id);
	if (!view) {
		qWarning() << "UiHelper::add_signals_to_xy_plot_view(): View not " <<
			"found: " << QString::fromStdString(view_id);
		return;
	}
	auto plot_view = dynamic_cast<ui::views::PlotView *>(view);
	if (!plot_view) {
		qWarning() << "UiHelper::add_signals_to_xy_plot_view(): View is not " <<
			"a plot view: " << QString::fromStdString(view_id);
		return;
	}

	plot_view->add_xy_curve(x_signal, y_signal);
}

} // namespace python
} // namespace sv

