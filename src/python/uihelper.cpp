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

#include <memory>
#include <string>

#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>

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
	if (!session_.main_window()) {
		Q_EMIT view_added("");
		return;
	}
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab) {
		Q_EMIT view_added("");
		return;
	}

	auto view = new ui::views::DataView(session_, signal);
	tab->add_view(view, area);
	Q_EMIT view_added(view->id());
}

void UiHelper::add_control_view(std::string device_id, Qt::DockWidgetArea area,
	shared_ptr<sv::devices::Configurable> configurable)
{
	if (!session_.main_window()) {
		Q_EMIT view_added("");
		return;
	}
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab) {
		Q_EMIT view_added("");
		return;
	}
	auto view = ui::views::viewhelper::get_view_for_configurable(
		session_, configurable);
	if (!view) {
		Q_EMIT view_added("");
		return;
	}

	tab->add_view(view, area);
	Q_EMIT view_added(view->id());
}

void UiHelper::add_plot_view(std::string device_id, Qt::DockWidgetArea area,
	shared_ptr<sv::channels::BaseChannel> channel)
{
	if (!session_.main_window()) {
		Q_EMIT view_added("");
		return;
	}
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab) {
		Q_EMIT view_added("");
		return;
	}

	auto view = new ui::views::PlotView(session_, channel);
	tab->add_view(view, area);
	Q_EMIT view_added(view->id());
}

void UiHelper::add_plot_view(std::string device_id, Qt::DockWidgetArea area,
	shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	if (!session_.main_window()) {
		Q_EMIT view_added("");
		return;
	}
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab) {
		Q_EMIT view_added("");
		return;
	}

	auto view = new ui::views::PlotView(session_, signal);
	tab->add_view(view, area);
	Q_EMIT view_added(view->id());
}

void UiHelper::add_plot_view(std::string device_id, Qt::DockWidgetArea area,
	shared_ptr<sv::data::AnalogTimeSignal> x_signal,
	shared_ptr<sv::data::AnalogTimeSignal> y_signal)
{
	if (!session_.main_window()) {
		Q_EMIT view_added("");
		return;
	}
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab) {
		Q_EMIT view_added("");
		return;
	}

	auto view = new ui::views::PlotView(session_, x_signal, y_signal);
	tab->add_view(view, area);
	Q_EMIT view_added(view->id());
}

void UiHelper::add_power_panel_view(std::string device_id,
	Qt::DockWidgetArea area,
	shared_ptr<sv::data::AnalogTimeSignal> voltage_signal,
	shared_ptr<sv::data::AnalogTimeSignal> current_signal)
{
	if (!session_.main_window()) {
		Q_EMIT view_added("");
		return;
	}
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab) {
		Q_EMIT view_added("");
		return;
	}

	auto view = new ui::views::PowerPanelView(
		session_, voltage_signal, current_signal);
	tab->add_view(view, area);
	Q_EMIT view_added(view->id());
}

void UiHelper::add_value_panel_view(std::string device_id,
	Qt::DockWidgetArea area, shared_ptr<sv::channels::BaseChannel> channel)
{
	if (!session_.main_window()) {
		Q_EMIT view_added("");
		return;
	}
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab) {
		Q_EMIT view_added("");
		return;
	}

	auto view = new ui::views::ValuePanelView(session_, channel);
	tab->add_view(view, area);
	Q_EMIT view_added(view->id());
}

void UiHelper::add_value_panel_view(std::string device_id,
	Qt::DockWidgetArea area, shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	if (!session_.main_window()) {
		Q_EMIT view_added("");
		return;
	}
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab) {
		Q_EMIT view_added("");
		return;
	}

	auto view = new ui::views::ValuePanelView(session_, signal);
	tab->add_view(view, area);
	Q_EMIT view_added(view->id());
}

void UiHelper::add_signal_to_data_view(std::string device_id,
	std::string view_id, shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	if (!session_.main_window())
		return;
	auto tab = session_.main_window()->get_base_tab_from_device_id(device_id);
	if (!tab)
		return;
	auto view = tab->get_view_from_view_id(view_id);
	if (!view)
		return;

	((ui::views::DataView *)view)->add_signal(signal);
}

void UiHelper::add_signal_to_plot_view(std::string device_id, std::string view_id,
	shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	if (!session_.main_window())
		return;
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
	auto plot_view = qobject_cast<ui::views::PlotView *>(view);
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
	if (!session_.main_window())
		return;
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
	auto plot_view = qobject_cast<ui::views::PlotView *>(view);
	if (!plot_view) {
		qWarning() << "UiHelper::add_signals_to_xy_plot_view(): View is not " <<
			"a plot view: " << QString::fromStdString(view_id);
		return;
	}

	plot_view->add_xy_curve(x_signal, y_signal);
}

void UiHelper::show_message_box(const std::string &title,
	const std::string &text)
{
	if (!session_.main_window()) {
		Q_EMIT message_box_canceled();
		return;
	}

	auto button = QMessageBox::information(session_.main_window(),
		QString::fromStdString(title), QString::fromStdString(text));

	if (button == QMessageBox::StandardButton::Ok)
		Q_EMIT message_box_finished();
	else
		Q_EMIT message_box_canceled();
}

void UiHelper::show_string_input_dialog(const std::string &title,
	const std::string &label, const std::string &value)
{
	if (!session_.main_window()) {
		Q_EMIT input_dialog_canceled();
		return;
	}

	bool ok;
	QString s = QInputDialog::getText(session_.main_window(),
		QString::fromStdString(title), QString::fromStdString(label),
		QLineEdit::Normal, QString::fromStdString(value), &ok,
		Qt::WindowFlags(), Qt::ImhNone);

	if (ok)
		Q_EMIT input_dialog_finished(QVariant(s));
	else
		Q_EMIT input_dialog_canceled();

}

void UiHelper::show_double_input_dialog(const std::string &title,
	const std::string &label, double value, int decimals, double step,
	double min, double max)
{
	if (!session_.main_window()) {
		Q_EMIT input_dialog_canceled();
		return;
	}

	bool ok;
	double d = QInputDialog::getDouble(session_.main_window(),
		QString::fromStdString(title), QString::fromStdString(label),
		value, min, max, decimals, &ok, Qt::WindowFlags(), step);

	if (ok)
		Q_EMIT input_dialog_finished(QVariant(d));
	else
		Q_EMIT input_dialog_canceled();
}

void UiHelper::show_int_input_dialog(const std::string &title,
	const std::string &label, int value, int step, int min, int max)
{
	if (!session_.main_window()) {
		Q_EMIT input_dialog_canceled();
		return;
	}

	bool ok;
	int i = QInputDialog::getInt(session_.main_window(),
		QString::fromStdString(title), QString::fromStdString(label),
		value, min, max, step, &ok, Qt::WindowFlags());

	if (ok)
		Q_EMIT input_dialog_finished(QVariant(i));
	else
		Q_EMIT input_dialog_canceled();
}


} // namespace python
} // namespace sv

