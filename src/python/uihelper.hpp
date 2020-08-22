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

#ifndef PYTHON_UIHELPER_HPP
#define PYTHON_UIHELPER_HPP

#include <memory>
#include <string>
#include <tuple>

#include <QDockWidget>
#include <QObject>
#include <QVariant>

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

namespace ui {
namespace tabs {
class BaseTab;
}
namespace views {
class BasePlotView;
class BaseView;
class TimePlotView;
}
}

namespace python {

class UiHelper : public QObject
{
	Q_OBJECT

public:
	explicit UiHelper(Session &session);

public Q_SLOTS:
	void add_device_tab(shared_ptr<sv::devices::BaseDevice> device);

	void add_data_view(const std::string &tab_id, Qt::DockWidgetArea area,
		shared_ptr<sv::data::AnalogTimeSignal> signal);
	void add_control_view(const std::string &tab_id, Qt::DockWidgetArea area,
		shared_ptr<sv::devices::Configurable> configurable);
	void add_time_plot_view(const std::string &tab_id, Qt::DockWidgetArea area);
	void add_xy_plot_view(const std::string &tab_id, Qt::DockWidgetArea area);
	void add_power_panel_view(const std::string &tab_id,
		Qt::DockWidgetArea area,
		shared_ptr<sv::data::AnalogTimeSignal> voltage_signal,
		shared_ptr<sv::data::AnalogTimeSignal> current_signal);
	void add_value_panel_view(const std::string &tab_id,
		Qt::DockWidgetArea area, shared_ptr<sv::channels::BaseChannel> channel);
	void add_value_panel_view(const std::string &tab_id,
		Qt::DockWidgetArea area, shared_ptr<sv::data::AnalogTimeSignal> signal);

	void add_signal_to_data_view(const std::string &tab_id,
		const std::string &view_id,
		shared_ptr<sv::data::AnalogTimeSignal> signal);

	void set_channel_to_time_plot_view(const std::string &tab_id,
		const std::string &view_id,
		shared_ptr<sv::channels::BaseChannel> channel);
	void add_curve_to_time_plot_view(const std::string &tab_id,
		const std::string &view_id,
		shared_ptr<sv::data::AnalogTimeSignal> signal);
	void add_curve_to_xy_plot_view(const std::string &tab_id,
		const std::string &view_id,
		shared_ptr<sv::data::AnalogTimeSignal> x_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_signal);
	void set_curve_name(const std::string &tab_id, const std::string &view_id,
		const std::string &curve_id, const std::string &name);
	void set_curve_color(const std::string &tab_id, const std::string &view_id,
		const std::string &curve_id, std::tuple<int, int, int> color);

	void show_message_box(const std::string &title, const std::string &text);
	void show_string_input_dialog(const std::string &title,
		const std::string &label, const std::string &value);
	void show_double_input_dialog(const std::string &title,
		const std::string &label, double value, int decimals, double step,
		double min, double max);
	void show_int_input_dialog(const std::string &title,
		const std::string &label, int value, int step, int min, int max);

private:
	Session &session_;

	ui::tabs::BaseTab *get_tab(const string &tab_id) const;
	ui::views::BaseView *get_view(const string &tab_id,
		const string &view_id) const;
	ui::views::BasePlotView *get_base_plot_view(const string &tab_id,
		const string &view_id) const;
	ui::views::TimePlotView *get_time_plot_view(const string &tab_id,
		const string &view_id) const;

Q_SIGNALS:
	void tab_added(const std::string &tab_id);
	void view_added(const std::string &view_id);
	void curve_added(const std::string &curve_id);
	void message_box_finished();
	void message_box_canceled();
	void input_dialog_finished(const QVariant &qvar_input);
	void input_dialog_canceled();

};

} // namespace python
} // namespace sv

#endif // PYTHON_UIHELPER_HPP
