/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <string>

#include <QMessageBox>
#include <QSettings>
#include <QUuid>
#include <QVariant>

#include "timeplotview.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/dialogs/selectsignaldialog.hpp"
#include "src/ui/views/baseplotview.hpp"
#include "src/ui/views/viewhelper.hpp"
#include <src/ui/widgets/plot/curve.hpp>
#include "src/ui/widgets/plot/plot.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"
#include "src/ui/widgets/plot/timecurvedata.hpp"

using std::dynamic_pointer_cast;
using std::static_pointer_cast;
using std::string;

Q_DECLARE_METATYPE(sv::ui::widgets::plot::BaseCurveData *)

namespace sv {
namespace ui {
namespace views {

TimePlotView::TimePlotView(Session &session, QUuid uuid, QWidget *parent) :
	BasePlotView(session, uuid, parent),
	channel_(nullptr)
{
	id_ = "timeplot:" + uuid_.toString(QUuid::WithoutBraces).toStdString();
	plot_type_ = PlotType::TimePlot;
}

QString TimePlotView::title() const
{
	QString title;

	if (channel_)
		title = tr("Channel");
	else
		title = tr("Signal");

	if (channel_)
		title = title.append(" ").append(channel_->display_name());
	else if (!plot_->curves().empty())
		title = title.append(" ").append(plot_->curves()[0]->name());

	return title;
}

void TimePlotView::set_channel(shared_ptr<channels::BaseChannel> channel)
{
	assert(channel);

	if (channel_) {
		disconnect(channel_.get(), &channels::BaseChannel::signal_added,
			this, &TimePlotView::on_signal_changed);
		disconnect(channel_.get(), &channels::BaseChannel::signal_changed,
			this, &TimePlotView::on_signal_changed);
	}

	// TODO: Remove old curves??

	channel_ = channel;

	shared_ptr<data::AnalogTimeSignal> signal;
	if (channel_->actual_signal())
		signal = static_pointer_cast<data::AnalogTimeSignal>(
			channel_->actual_signal());
	if (signal)
		add_signal(signal);

	connect(channel_.get(), &channels::BaseChannel::signal_added,
		this, &TimePlotView::on_signal_changed);
	connect(channel_.get(), &channels::BaseChannel::signal_changed,
		this, &TimePlotView::on_signal_changed);

	Q_EMIT title_changed();
}

string TimePlotView::add_signal(shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	assert(signal);
	string id = "";

	// Check if new actual_signal is already added to this plot
	for (const auto &curve : plot_->curves()) {
		auto curve_data =
			qobject_cast<widgets::plot::TimeCurveData *>(curve->curve_data());
		if (!curve_data)
			continue;
		if (curve_data->signal() == signal)
			return id;
	}

	auto curve = new widgets::plot::TimeCurveData(signal);
	id = plot_->add_curve(curve);
	if (!id.empty()) {
		update_add_marker_menu();
		Q_EMIT title_changed();
	}
	else {
		QMessageBox::warning(this,
			tr("Cannot add signal"), tr("Cannot add time signal to plot!"),
			QMessageBox::Ok);
	}

	return id;
}

void TimePlotView::save_settings(QSettings &settings) const
{
	BasePlotView::save_settings(settings);

	// TODO: Can the channel be saved inside the plot widget?
	bool save_curves = true;
	if (channel_) {
		SettingsManager::save_channel(channel_, settings);
		save_curves = false;
	}
	plot_->save_settings(settings, save_curves);
}

void TimePlotView::restore_settings(QSettings &settings)
{
	BasePlotView::restore_settings(settings);

	// TODO: Can the channel be restored inside the plot widget?
	bool restore_curves = true;
	auto channel = SettingsManager::restore_channel(session_, settings);
	if (channel) {
		set_channel(channel);
		restore_curves = false;
	}
	plot_->restore_settings(settings, restore_curves);
	update_add_marker_menu();
}

void TimePlotView::on_action_add_signal_triggered()
{
	shared_ptr<sv::devices::BaseDevice> selected_device;
	if (channel_)
		selected_device = channel_->parent_device();

	ui::dialogs::SelectSignalDialog dlg(session(), selected_device);
	if (!dlg.exec())
		return;

	for (const auto &signal : dlg.signals()) {
		add_signal(dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal));
	}
}

void TimePlotView::on_signal_changed()
{
	if (!channel_)
		return;

	shared_ptr<sv::data::AnalogTimeSignal> signal;
	if (channel_->actual_signal())
		signal = dynamic_pointer_cast<sv::data::AnalogTimeSignal>(
			channel_->actual_signal());
	if (signal)
		add_signal(signal);
}

} // namespace views
} // namespace ui
} // namespace sv
