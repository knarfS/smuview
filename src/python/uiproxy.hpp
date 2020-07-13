/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef PYTHON_UIPROXY_HPP
#define PYTHON_UIPROXY_HPP

#include <memory>
#include <string>
#include <tuple>
#include <pybind11/pybind11.h>

#include <QDockWidget>
#include <QEventLoop>
#include <QMetaObject>
#include <QObject>
#include <QTimer>
#include <QVariant>

using std::shared_ptr;
using std::string;
using std::tuple;

namespace py = pybind11;

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

/**
 * The python interpreter is executed in its own thread, therefore calling any
 * methods that will manipulate Qt widgets directly won't work.
 * So we are using UiProxy which is communicating with the Qt main loop via
 * signals and slots.
 */
class UiProxy : public QObject
{
	Q_OBJECT

public:
	UiProxy(Session &session, shared_ptr<UiHelper> ui_helper);

	string ui_add_device_tab(shared_ptr<devices::BaseDevice> device);

	string ui_add_data_view(string tab_id, Qt::DockWidgetArea area,
		shared_ptr<data::AnalogTimeSignal> signal);
	string ui_add_control_view(string tab_id, Qt::DockWidgetArea area,
		shared_ptr<devices::Configurable> configurable);
	string ui_add_time_plot_view(string tab_id, Qt::DockWidgetArea area);
	string ui_add_xy_plot_view(string tab_id, Qt::DockWidgetArea area);
	string ui_add_power_panel_view(string tab_id, Qt::DockWidgetArea area,
		shared_ptr<data::AnalogTimeSignal> voltage_signal,
		shared_ptr<data::AnalogTimeSignal> current_signal);
	string ui_add_value_panel_view(string tab_id, Qt::DockWidgetArea area,
		shared_ptr<channels::BaseChannel> channel);
	string ui_add_value_panel_view(string tab_id, Qt::DockWidgetArea area,
		shared_ptr<data::AnalogTimeSignal> signal);

	void ui_add_signal_to_data_view(string tab_id, string view_id,
		shared_ptr<data::AnalogTimeSignal> signal);

	void ui_set_channel_to_time_plot_view(string tab_id, string view_id,
		shared_ptr<channels::BaseChannel> channel);
	string ui_add_curve_to_time_plot_view(string tab_id, string view_id,
		shared_ptr<data::AnalogTimeSignal> signal);
	string ui_add_curve_to_xy_plot_view(string tab_id, string view_id,
		shared_ptr<data::AnalogTimeSignal> x_signal,
		shared_ptr<data::AnalogTimeSignal> y_signal);
	void ui_set_curve_name(string tab_id, string view_id, string curve_id,
		string name);
	void ui_set_curve_color(string tab_id, string view_id, string curve_id,
		tuple<int, int, int> color);

	bool ui_show_message_box(const std::string &title, const std::string &text);
	py::object ui_show_string_input_dialog(const string &title,
		const string &label, const string &value);
	py::object ui_show_double_input_dialog(const string &title,
		const string &label, double value, int decimals, double step,
		double min, double max);
	py::object ui_show_int_input_dialog(const string &title,
		const string &label, int value, int step, int min, int max);

private:
	void init_wait_for_tab_added(string &id, int timeout = 1000);
	void init_wait_for_view_added(string &id, int timeout = 1000);
	void init_wait_for_curve_added(string &id, int timeout = 1000);
	void init_wait_for_message_box(bool &ok, int timeout = 0);
	void init_wait_for_input_dialog(bool &ok, QVariant &qvar, int timeout = 0);
	void finish_wait_for_signal();

	Session &session_;
	shared_ptr<UiHelper> ui_helper_;
	QEventLoop event_loop_;
	QTimer timer_;
	QMetaObject::Connection event_loop_finished_conn_;
	QMetaObject::Connection event_loop_canceled_conn_;
	QMetaObject::Connection timer_conn_;

Q_SIGNALS:
	void add_device_tab(shared_ptr<sv::devices::BaseDevice> device);

	void add_data_view(std::string tab_id, Qt::DockWidgetArea area,
		shared_ptr<sv::data::AnalogTimeSignal> signal);
	void add_control_view(std::string tab_id, Qt::DockWidgetArea area,
		shared_ptr<sv::devices::Configurable> configurable);
	void add_time_plot_view(std::string tab_id, Qt::DockWidgetArea area);
	void add_xy_plot_view(std::string tab_id, Qt::DockWidgetArea area);
	void add_power_panel_view(std::string tab_id, Qt::DockWidgetArea area,
		shared_ptr<sv::data::AnalogTimeSignal> voltage_signal,
		shared_ptr<sv::data::AnalogTimeSignal> current_signal);
	void add_value_panel_view(std::string tab_id, Qt::DockWidgetArea area,
		shared_ptr<sv::channels::BaseChannel> channel);
	void add_value_panel_view(std::string tab_id, Qt::DockWidgetArea area,
		shared_ptr<sv::data::AnalogTimeSignal> signal);

	void add_signal_to_data_view(std::string tab_id, std::string view_id,
		shared_ptr<sv::data::AnalogTimeSignal> signal);

	void set_channel_to_time_plot_view(std::string tab_id, std::string view_id,
		shared_ptr<sv::channels::BaseChannel> channel);
	void add_curve_to_time_plot_view(std::string tab_id, std::string view_id,
		shared_ptr<sv::data::AnalogTimeSignal> signal);
	void add_curve_to_xy_plot_view(std::string tab_id, std::string view_id,
		shared_ptr<sv::data::AnalogTimeSignal> x_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_signal);
	void set_curve_name(std::string tab_id, std::string view_id,
		std::string curve_id, std::string name);
	void set_curve_color(std::string tab_id, std::string view_id,
		std::string curve_id, std::tuple<int, int, int> color);

	void show_message_box(const std::string &title, const std::string &text);
	void show_string_input_dialog(const std::string &title,
		const std::string &label, const std::string &value);
	void show_double_input_dialog(const std::string &title,
		const std::string &label, double value, int decimals, double step,
		double min, double max);
	void show_int_input_dialog(const std::string &title,
		const std::string &label, int value, int step, int min, int max);
};

} // namespace python
} // namespace sv

#endif // PYTHON_UIPROXY_HPP
