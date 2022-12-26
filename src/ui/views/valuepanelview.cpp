/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2022 Frank Stettner <frank-stettner@gmx.net>
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
#include <memory>
#include <string>
#include <utility>

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QSettings>
#include <QTimer>
#include <QUuid>
#include <QVariant>
#include <QVBoxLayout>

#include "valuepanelview.hpp"
#include "src/data/datautil.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/viewhelper.hpp"
#include "src/ui/widgets/monofontdisplay.hpp"

using std::dynamic_pointer_cast;
using std::shared_ptr;

namespace sv {
namespace ui {
namespace views {

ValuePanelView::ValuePanelView(Session &session, QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	channel_(nullptr),
	signal_(nullptr),
	value_min_(std::numeric_limits<double>::max()),
	value_max_(std::numeric_limits<double>::lowest()),
	action_reset_display_(new QAction(this))
{
	id_ = "valuepanel:" + util::format_uuid(uuid_);

	setup_ui();
	setup_toolbar();
	reset_display();

	timer_ = new QTimer(this);
	init_timer();
}

ValuePanelView::~ValuePanelView()
{
	stop_timer();
}

QString ValuePanelView::title() const
{
	QString title;

	if (channel_)
		title = tr("Channel");
	else
		title = tr("Signal");

	if (channel_)
		title = title.append(" ").append(channel_->display_name());
	else if (signal_)
		title = title.append(" ").append(signal_->display_name());

	return title;
}

void ValuePanelView::set_channel(shared_ptr<channels::BaseChannel> channel)
{
	assert(channel);

	disconnect_signals_channel();
	disconnect_signals_signal();

	channel_ = channel;
	signal_ = dynamic_pointer_cast<sv::data::AnalogTimeSignal>(
		channel_->actual_signal());
	if (signal_) {
		init_displays();
		connect_signals_signal();
	}

	connect_signals_channel();

	Q_EMIT title_changed();
}

void ValuePanelView::set_signal(shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	assert(signal);

	disconnect_signals_channel();
	disconnect_signals_signal();

	channel_ = nullptr;
	signal_ = signal;
	init_displays();

	connect_signals_signal();

	Q_EMIT title_changed();
}

void ValuePanelView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();
	QGridLayout *panel_layout = new QGridLayout();

	value_display_ = new widgets::MonoFontDisplay(
		widgets::MonoFontDisplayType::AutoRangeWithSRDigits, "", "", "", false);
	value_min_display_ = new widgets::MonoFontDisplay(
		widgets::MonoFontDisplayType::AutoRange, "", "",
		data::datautil::format_quantity_flag(data::QuantityFlag::Min), true);
	value_max_display_ = new widgets::MonoFontDisplay(
		widgets::MonoFontDisplayType::AutoRange, "", "",
		data::datautil::format_quantity_flag(data::QuantityFlag::Max), true);

	panel_layout->addWidget(value_display_, 0, 0, 1, 2, Qt::AlignHCenter);
	panel_layout->addWidget(value_min_display_, 1, 0, 1, 1, Qt::AlignHCenter);
	panel_layout->addWidget(value_max_display_, 1, 1, 1, 1, Qt::AlignHCenter);
	layout->addLayout(panel_layout);
	layout->addStretch(1);

	this->central_widget_->setLayout(layout);
}

void ValuePanelView::setup_toolbar()
{
	action_reset_display_->setText(tr("Reset display"));
	action_reset_display_->setIcon(
		QIcon::fromTheme("view-refresh",
		QIcon(":/icons/view-refresh.png")));
	connect(action_reset_display_, &QAction::triggered,
		this, &ValuePanelView::on_action_reset_display_triggered);

	toolbar_ = new QToolBar("Panel Toolbar");
	toolbar_->addAction(action_reset_display_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void ValuePanelView::init_displays()
{
	QString unit = signal_->unit_name();
	QString unit_suffix;
	set<sv::data::QuantityFlag> quantity_flags = signal_->quantity_flags();
	int total_digits = signal_->total_digits();
	int sr_digits = signal_->sr_digits();

	if (quantity_flags.count(sv::data::QuantityFlag::AC) > 0) {
		//unit_suffix = QString::fromUtf8("\u23E6");
		unit_suffix = sv::data::datautil::format_quantity_flag(
			sv::data::QuantityFlag::AC);
		quantity_flags.erase(sv::data::QuantityFlag::AC);
	}
	else if (quantity_flags.count(sv::data::QuantityFlag::DC) > 0) {
		//unit_suffix = QString::fromUtf8("\u2393");
		unit_suffix = sv::data::datautil::format_quantity_flag(
			sv::data::QuantityFlag::DC);
		quantity_flags.erase(sv::data::QuantityFlag::DC);
	}
	set<sv::data::QuantityFlag> quantity_flags_min = quantity_flags;
	quantity_flags_min.insert(sv::data::QuantityFlag::Min);
	set<sv::data::QuantityFlag> quantity_flags_max = quantity_flags;
	quantity_flags_max.insert(sv::data::QuantityFlag::Max);

	value_display_->set_unit(unit);
	value_display_->set_unit_suffix(unit_suffix);
	value_display_->set_extra_text(
		sv::data::datautil::format_quantity_flags(quantity_flags, "\n"));
	value_display_->set_sr_digits(total_digits, sr_digits);

	value_min_display_->set_unit(unit);
	value_min_display_->set_unit_suffix(unit_suffix);
	value_min_display_->set_extra_text(
		sv::data::datautil::format_quantity_flags(quantity_flags_min, "\n"));
	value_min_display_->set_decimal_places(
		sv::data::DefaultTotalDigits, sv::data::DefaultDecimalPlaces);

	value_max_display_->set_unit(unit);
	value_max_display_->set_unit_suffix(unit_suffix);
	value_max_display_->set_extra_text(
		sv::data::datautil::format_quantity_flags(quantity_flags_max, "\n"));
	value_max_display_->set_decimal_places(
		sv::data::DefaultTotalDigits, sv::data::DefaultDecimalPlaces);
}

void ValuePanelView::connect_signals_channel()
{
	if (!channel_)
		return;

	connect(channel_.get(), &channels::BaseChannel::signal_added,
		this, &ValuePanelView::on_signal_changed);
	connect(channel_.get(), &channels::BaseChannel::signal_changed,
		this, &ValuePanelView::on_signal_changed);
}

void ValuePanelView::disconnect_signals_channel()
{
	if (!channel_)
		return;

	disconnect(channel_.get(), &channels::BaseChannel::signal_added,
		this, &ValuePanelView::on_signal_changed);
	disconnect(channel_.get(), &channels::BaseChannel::signal_changed,
		this, &ValuePanelView::on_signal_changed);
}

void ValuePanelView::connect_signals_signal()
{
	if (!signal_)
		return;

	//connect(signal_.get(), SIGNAL(unit_changed(QString)),
	//	value_display_, SLOT(set_unit(const String)));
	connect(signal_.get(), &data::AnalogTimeSignal::digits_changed,
		value_display_, &widgets::MonoFontDisplay::set_sr_digits);
}

void ValuePanelView::disconnect_signals_signal()
{
	if (!signal_)
		return;

	//disconnect(signal_.get(), SIGNAL(unit_changed(QString)),
	//	value_display_, SLOT(set_unit(QString)));
	disconnect(signal_.get(), &data::AnalogTimeSignal::digits_changed,
		value_display_, &widgets::MonoFontDisplay::set_sr_digits);
}

void ValuePanelView::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	BaseView::save_settings(settings, origin_device);

	if (signal_)
		SettingsManager::save_signal(signal_, settings, origin_device);
	else
		SettingsManager::save_channel(channel_, settings, origin_device);
}

void ValuePanelView::restore_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	BaseView::restore_settings(settings, origin_device);

	auto signal = SettingsManager::restore_signal(
		session_, settings, origin_device);
	if (signal) {
		set_signal(
			dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal));
		return;
	}

	auto channel = SettingsManager::restore_channel(
		session_, settings, origin_device);
	if (channel)
		set_channel(channel);
}

void ValuePanelView::reset_display()
{
	value_display_->reset_value();
	value_min_display_->reset_value();
	value_max_display_->reset_value();
}

void ValuePanelView::init_timer()
{
	value_min_ = std::numeric_limits<double>::max();
	value_max_ = std::numeric_limits<double>::lowest();

	connect(timer_, &QTimer::timeout, this, &ValuePanelView::on_update);
	timer_->start(250);
}

void ValuePanelView::stop_timer()
{
	if (!timer_->isActive())
		return;

	timer_->stop();
	disconnect(timer_, &QTimer::timeout, this, &ValuePanelView::on_update);

	reset_display();
}

void ValuePanelView::on_update()
{
	if (!signal_ || signal_->sample_count() == 0)
		return;

	double value = signal_->last_value();
	if (value_min_ > value)
		value_min_ = value;
	if (value_max_ < value)
		value_max_ = value;

	value_display_->set_value(value);
	value_min_display_->set_value(value_min_);
	value_max_display_->set_value(value_max_);
}

void ValuePanelView::on_signal_changed()
{
	// When channel_ is not set, we have a fixed signal_ and nothing will change
	if (!channel_)
		return;

	disconnect_signals_signal();

	signal_ = dynamic_pointer_cast<sv::data::AnalogTimeSignal>(
		channel_->actual_signal());
	if (!signal_)
		return;
	init_displays();

	connect_signals_signal();

	Q_EMIT title_changed();
}

void ValuePanelView::on_action_reset_display_triggered()
{
	stop_timer();
	init_timer();
}

} // namespace views
} // namespace ui
} // namespace sv
