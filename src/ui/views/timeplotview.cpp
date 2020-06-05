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

#include <QMessageBox>
#include <QSettings>
#include <QUuid>

#include "timeplotview.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/dialogs/selectsignaldialog.hpp"
#include "src/ui/views/baseplotview.hpp"
#include "src/ui/views/viewhelper.hpp"
#include "src/ui/widgets/plot/plot.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"
#include "src/ui/widgets/plot/timecurvedata.hpp"

using std::dynamic_pointer_cast;
using std::static_pointer_cast;

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
	else if (!curves_.empty())
		title = title.append(" ").append(curves_[0]->name());

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

void TimePlotView::add_signal(shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	assert(signal);

	// Check if new actual_signal is already added to this plot
	for (const auto &curve : curves_) {
		if (((widgets::plot::TimeCurveData *)curve)->signal() == signal)
			return;
	}

	auto curve = new widgets::plot::TimeCurveData(signal);
	if (plot_->add_curve(curve)) {
		curves_.push_back(curve);
		update_add_marker_menu();
		Q_EMIT title_changed();
	}
	else {
		QMessageBox::warning(this,
			tr("Cannot add signal"), tr("Cannot add time signal to plot!"),
			QMessageBox::Ok);
	}
}

void TimePlotView::save_settings(QSettings &settings) const
{
	BasePlotView::save_settings(settings);

	if (channel_) {
		viewhelper::save_channel(channel_, settings);
		return;
	}

	// No channel used, save all signals.
	size_t i = 0;
	for (const auto &curve : curves_) {
		settings.beginGroup(QString("curve%1").arg(i++));
		auto t_curve = static_cast<widgets::plot::TimeCurveData *>(curve);
		viewhelper::save_signal(t_curve->signal(), settings);
		settings.endGroup();
	}
}

void TimePlotView::restore_settings(QSettings &settings)
{
	BasePlotView::restore_settings(settings);

	auto channel = viewhelper::restore_channel(session_, settings);
	if (channel) {
		set_channel(channel);
		return;
	}

	// No channel used, restore all signals.
	for (const auto &group : settings.childGroups()) {
		if (!group.startsWith("curve"))
			continue;
		settings.beginGroup(group);
		auto signal = viewhelper::restore_signal(session_, settings);
		if (signal)
			add_signal(dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal));
		settings.endGroup();
	}
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
