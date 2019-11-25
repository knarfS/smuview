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

#ifndef PYTHON_UIPROXY_HPP
#define PYTHON_UIPROXY_HPP

#include <memory>
#include <string>

#include <QDockWidget>
#include <QObject>

using std::shared_ptr;
using std::string;

namespace sv {

class Session;

namespace channels {
class BaseChannel;
}
namespace data {
class AnalogTimeSignal;
}
namespace devices {
class BaseDevice;
class Configurable;
}

namespace python {

class UiHelper;

class UiProxy : public QObject
{
	Q_OBJECT

public:
	UiProxy(Session &session, shared_ptr<UiHelper> ui_helper);

	void ui_add_device_tab(shared_ptr<devices::BaseDevice> device);

	string ui_add_data_view(string device_id, Qt::DockWidgetArea area,
		shared_ptr<data::AnalogTimeSignal> signal);
	string ui_add_control_view(string device_id, Qt::DockWidgetArea area,
		shared_ptr<devices::Configurable> configurable);
	string ui_add_plot_view(string device_id, Qt::DockWidgetArea area,
		shared_ptr<channels::BaseChannel> channel);
	string ui_add_plot_view(string device_id, Qt::DockWidgetArea area,
		shared_ptr<data::AnalogTimeSignal> signal);
	string ui_add_plot_view(string device_id, Qt::DockWidgetArea area,
		shared_ptr<data::AnalogTimeSignal> x_signal,
		shared_ptr<data::AnalogTimeSignal> y_signal);
	string ui_add_power_panel_view(string device_id, Qt::DockWidgetArea area,
		shared_ptr<data::AnalogTimeSignal> voltage_signal,
		shared_ptr<data::AnalogTimeSignal> current_signal);
	string ui_add_value_panel_view(string device_id, Qt::DockWidgetArea area,
		shared_ptr<channels::BaseChannel> channel);
	string ui_add_value_panel_view(string device_id, Qt::DockWidgetArea area,
		shared_ptr<data::AnalogTimeSignal> signal);
	void ui_add_signal_to_plot(string device_id, string view_id,
		shared_ptr<data::AnalogTimeSignal> signal);
	void ui_add_y_signal_to_xy_plot(string device_id, string view_id,
		shared_ptr<data::AnalogTimeSignal> y_signal);
	void ui_add_signals_to_xy_plot(string device_id, string view_id,
		shared_ptr<data::AnalogTimeSignal> x_signal,
		shared_ptr<data::AnalogTimeSignal> y_signal);

private:
	Session &session_;
	shared_ptr<UiHelper> ui_helper_;

Q_SIGNALS:
	void add_device_tab(shared_ptr<sv::devices::BaseDevice>);

	void add_data_view(std::string device_id, Qt::DockWidgetArea area,
		shared_ptr<sv::data::AnalogTimeSignal> signal);
	void add_control_view(std::string device_id, Qt::DockWidgetArea area,
		shared_ptr<sv::devices::Configurable> configurable);
	void add_plot_view(std::string device_id, Qt::DockWidgetArea area,
		shared_ptr<sv::channels::BaseChannel> channel);
	void add_plot_view(std::string device_id, Qt::DockWidgetArea area,
		shared_ptr<sv::data::AnalogTimeSignal> signal);
	void add_plot_view(std::string device_id, Qt::DockWidgetArea area,
		shared_ptr<sv::data::AnalogTimeSignal> x_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_signal);
	void add_power_panel_view(std::string device_id, Qt::DockWidgetArea area,
		shared_ptr<sv::data::AnalogTimeSignal> voltage_signal,
		shared_ptr<sv::data::AnalogTimeSignal> current_signal);
	void add_value_panel_view(std::string device_id, Qt::DockWidgetArea area,
		shared_ptr<sv::channels::BaseChannel> channel);
	void add_value_panel_view(std::string device_id, Qt::DockWidgetArea area,
		shared_ptr<sv::data::AnalogTimeSignal> signal);

	void add_signal_to_plot(std::string device_id, std::string view_id,
		shared_ptr<sv::data::AnalogTimeSignal> signal);
	void add_y_signal_to_xy_plot(std::string device_id, std::string view_id,
		shared_ptr<sv::data::AnalogTimeSignal> y_signal);
	void add_signals_to_xy_plot(std::string device_id, std::string view_id,
		shared_ptr<sv::data::AnalogTimeSignal> x_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_signal);

};

} // namespace python
} // namespace sv

#endif // PYTHON_UIPROXY_HPP
