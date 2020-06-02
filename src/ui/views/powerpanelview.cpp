/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <set>
#include <string>

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>

#include "powerpanelview.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogbasesignal.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/viewhelper.hpp"
#include "src/ui/widgets/monofontdisplay.hpp"

using std::set;
using sv::data::QuantityFlag;

namespace sv {
namespace ui {
namespace views {

PowerPanelView::PowerPanelView(Session &session, QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	voltage_signal_(nullptr),
	current_signal_(nullptr),
	voltage_min_(std::numeric_limits<double>::max()),
	voltage_max_(std::numeric_limits<double>::lowest()),
	current_min_(std::numeric_limits<double>::max()),
	current_max_(std::numeric_limits<double>::lowest()),
	resistance_min_(std::numeric_limits<double>::max()),
	resistance_max_(std::numeric_limits<double>::lowest()),
	power_min_(std::numeric_limits<double>::max()),
	power_max_(std::numeric_limits<double>::lowest()),
	actual_amp_hours_(0),
	actual_watt_hours_(0),
	action_reset_displays_(new QAction(this))
{
	id_ = "powerpanel:" + uuid_.toString(QUuid::WithoutBraces).toStdString();

	setup_ui();
	setup_toolbar();
	connect_signals();
	reset_displays();

	timer_ = new QTimer(this);
	init_timer();
}

PowerPanelView::~PowerPanelView()
{
	stop_timer();
}

QString PowerPanelView::title() const
{
	QString title = tr("Power Panel");
	if (voltage_signal_ && current_signal_)
		title = title.append(" ").append(voltage_signal_->display_name()).
			append(" / ").append(current_signal_->display_name());

	return title;
}

void PowerPanelView::set_signals(
	shared_ptr<sv::data::AnalogTimeSignal> voltage_signal,
	shared_ptr<sv::data::AnalogTimeSignal> current_signal)
{
	assert(voltage_signal);
	assert(current_signal);

	disconnect_signals();
	stop_timer();
	voltage_signal_ = voltage_signal;
	current_signal_ = current_signal;
	init_timer();
	init_displays();
	connect_signals();

	Q_EMIT title_changed();
}

void PowerPanelView::setup_ui()
{
	int digits = 7;
	int decimal_places = 3;

	QVBoxLayout *layout = new QVBoxLayout();
	QGridLayout *panel_layout = new QGridLayout();

	voltage_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, false, "", "", "", false);
	voltage_min_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, false, "", "",
		data::datautil::format_quantity_flag(data::QuantityFlag::Min), true);
	voltage_max_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, false, "", "",
		data::datautil::format_quantity_flag(data::QuantityFlag::Max), true);

	current_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, false, "", "", "", false);
	current_min_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, false, "", "",
		data::datautil::format_quantity_flag(data::QuantityFlag::Min), true);
	current_max_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, false, "", "",
		data::datautil::format_quantity_flag(data::QuantityFlag::Max), true);

	resistance_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, true,
		data::datautil::format_unit(data::Unit::Ohm), "", "", false);
	resistance_min_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, true,
		data::datautil::format_unit(data::Unit::Ohm), "",
		data::datautil::format_quantity_flag(QuantityFlag::Min), true);
	resistance_max_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, true,
		data::datautil::format_unit(data::Unit::Ohm), "",
		data::datautil::format_quantity_flag(QuantityFlag::Max), true);

	power_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, true,
		data::datautil::format_unit(data::Unit::Watt), "", "", false);
	power_min_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, true,
		data::datautil::format_unit(data::Unit::Watt), "",
		data::datautil::format_quantity_flag(QuantityFlag::Min), true);
	power_max_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, true,
		data::datautil::format_unit(data::Unit::Watt), "",
		data::datautil::format_quantity_flag(QuantityFlag::Max), true);

	amp_hour_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, true,
		data::datautil::format_unit(data::Unit::AmpereHour), "", "", false);
	watt_hour_display_ = new widgets::MonoFontDisplay(
		digits, decimal_places, true,
		data::datautil::format_unit(data::Unit::WattHour), "", "", false);

	panel_layout->addWidget(voltage_display_, 0, 0, 1, 2, Qt::AlignHCenter);
	panel_layout->addWidget(voltage_min_display_, 1, 0, 1, 1, Qt::AlignHCenter);
	panel_layout->addWidget(voltage_max_display_, 1, 1, 1, 1, Qt::AlignHCenter);

	panel_layout->addWidget(current_display_, 2, 0, 1, 2, Qt::AlignHCenter);
	panel_layout->addWidget(current_min_display_, 3, 0, 1, 1, Qt::AlignHCenter);
	panel_layout->addWidget(current_max_display_, 3, 1, 1, 1, Qt::AlignHCenter);

	panel_layout->addWidget(resistance_display_, 0, 2, 1, 2, Qt::AlignHCenter);
	panel_layout->addWidget(resistance_min_display_, 1, 2, 1, 1, Qt::AlignHCenter);
	panel_layout->addWidget(resistance_max_display_, 1, 3, 1, 1, Qt::AlignHCenter);

	panel_layout->addWidget(power_display_, 2, 2, 1, 2, Qt::AlignHCenter);
	panel_layout->addWidget(power_min_display_, 3, 2, 1, 1, Qt::AlignHCenter);
	panel_layout->addWidget(power_max_display_, 3, 3, 1, 1, Qt::AlignHCenter);

	panel_layout->addWidget(amp_hour_display_, 0, 4, 2, 1, Qt::AlignCenter);
	panel_layout->addWidget(watt_hour_display_, 2, 4, 2, 1, Qt::AlignCenter);

	layout->addLayout(panel_layout);
	layout->addStretch(1);

	this->central_widget_->setLayout(layout);
}

void PowerPanelView::setup_toolbar()
{
	action_reset_displays_->setText(tr("Reset displays"));
	action_reset_displays_->setIcon(
		QIcon::fromTheme("view-refresh",
		QIcon(":/icons/view-refresh.png")));
	connect(action_reset_displays_, &QAction::triggered,
		this, &PowerPanelView::on_action_reset_displays_triggered);

	toolbar_ = new QToolBar("Power Panel Toolbar");
	toolbar_->addAction(action_reset_displays_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void PowerPanelView::init_displays()
{
	QString voltage_unit_suffix("");
	set<QuantityFlag> voltage_qfs = voltage_signal_->quantity_flags();
	if (voltage_qfs.count(QuantityFlag::AC) > 0) {
		//voltage_unit_suffix = QString::fromUtf8("\u23E6");
		voltage_unit_suffix = sv::data::datautil::format_quantity_flag(
			QuantityFlag::AC);
		voltage_qfs.erase(QuantityFlag::AC);
	}
	else if (voltage_qfs.count(QuantityFlag::DC) > 0) {
		//voltage_unit_suffix = QString::fromUtf8("\u2393");
		voltage_unit_suffix = sv::data::datautil::format_quantity_flag(
			QuantityFlag::DC);
		voltage_qfs.erase(QuantityFlag::DC);
	}
	set<QuantityFlag> voltage_qfs_min = voltage_qfs;
	voltage_qfs_min.insert(QuantityFlag::Min);
	set<QuantityFlag> voltage_qfs_max = voltage_qfs;
	voltage_qfs_max.insert(QuantityFlag::Max);

	voltage_display_->set_unit(voltage_signal_->unit_name());
	voltage_display_->set_unit_suffix(voltage_unit_suffix);
	voltage_display_->set_extra_text(
		sv::data::datautil::format_quantity_flags(voltage_qfs, "\n"));
	voltage_display_->set_digits(
		voltage_signal_->digits(), voltage_signal_->decimal_places());

	voltage_min_display_->set_unit(voltage_signal_->unit_name());
	voltage_min_display_->set_unit_suffix(voltage_unit_suffix);
	voltage_min_display_->set_extra_text(
		sv::data::datautil::format_quantity_flags(voltage_qfs_min, "\n"));
	voltage_min_display_->set_digits(
		voltage_signal_->digits(), voltage_signal_->decimal_places());

	voltage_max_display_->set_unit(voltage_signal_->unit_name());
	voltage_max_display_->set_unit_suffix(voltage_unit_suffix);
	voltage_max_display_->set_extra_text(
		sv::data::datautil::format_quantity_flags(voltage_qfs_max, "\n"));
	voltage_max_display_->set_digits(
		voltage_signal_->digits(), voltage_signal_->decimal_places());

	QString current_unit_suffix("");
	set<QuantityFlag> current_qfs = current_signal_->quantity_flags();
	if (current_qfs.count(QuantityFlag::AC) > 0) {
		current_unit_suffix = sv::data::datautil::format_quantity_flag(
			QuantityFlag::AC);
		current_qfs.erase(QuantityFlag::AC);
	}
	else if (current_qfs.count(QuantityFlag::DC) > 0) {
		current_unit_suffix = sv::data::datautil::format_quantity_flag(
			QuantityFlag::DC);
		current_qfs.erase(QuantityFlag::DC);
	}
	set<QuantityFlag> current_qfs_min = current_qfs;
	current_qfs_min.insert(QuantityFlag::Min);
	set<QuantityFlag> current_qfs_max = current_qfs;
	current_qfs_max.insert(QuantityFlag::Max);

	current_display_->set_unit(current_signal_->unit_name());
	current_display_->set_unit_suffix(current_unit_suffix);
	current_display_->set_extra_text(
		sv::data::datautil::format_quantity_flags(current_qfs, "\n"));
	current_display_->set_digits(
		current_signal_->digits(), current_signal_->decimal_places());

	current_min_display_->set_unit(current_signal_->unit_name());
	current_min_display_->set_unit_suffix(current_unit_suffix);
	current_min_display_->set_extra_text(
		sv::data::datautil::format_quantity_flags(current_qfs_min, "\n"));
	current_min_display_->set_digits(
		current_signal_->digits(), current_signal_->decimal_places());

	current_max_display_->set_unit(current_signal_->unit_name());
	current_max_display_->set_unit_suffix(current_unit_suffix);
	current_max_display_->set_extra_text(
		sv::data::datautil::format_quantity_flags(current_qfs_max, "\n"));
	current_max_display_->set_digits(
		current_signal_->digits(), current_signal_->decimal_places());

	// Use the smaller digits count to save space.
	int digits;
	if (voltage_signal_->digits() > current_signal_->digits())
		digits = current_signal_->digits();
	else
		digits = voltage_signal_->digits();
	// Use the smaller decimal_places count to save space.
	int decimal_places;
	if (voltage_signal_->decimal_places() > current_signal_->decimal_places())
		decimal_places = current_signal_->decimal_places();
	else
		decimal_places = voltage_signal_->decimal_places();

	resistance_display_->set_digits(digits, decimal_places);
	resistance_min_display_->set_digits(digits, decimal_places);
	resistance_max_display_->set_digits(digits, decimal_places);

	power_display_->set_digits(digits, decimal_places);
	power_min_display_->set_digits(digits, decimal_places);
	power_max_display_->set_digits(digits, decimal_places);

	amp_hour_display_->set_digits(digits, decimal_places);
	watt_hour_display_->set_digits(digits, decimal_places);
}

void PowerPanelView::connect_signals()
{
	if (!voltage_signal_ || !current_signal_)
		return;

	connect(voltage_signal_.get(), &data::AnalogBaseSignal::digits_changed,
		this, &PowerPanelView::on_digits_changed);
	connect(current_signal_.get(), &data::AnalogBaseSignal::digits_changed,
		this, &PowerPanelView::on_digits_changed);
}

void PowerPanelView::disconnect_signals()
{
	if (voltage_signal_) {
		disconnect(
			voltage_signal_.get(), &data::AnalogBaseSignal::digits_changed,
			this, &PowerPanelView::on_digits_changed);
	}
	if (current_signal_) {
		disconnect(
			current_signal_.get(), &data::AnalogBaseSignal::digits_changed,
			this, &PowerPanelView::on_digits_changed);
	}
}

void PowerPanelView::save_settings(QSettings &settings) const
{
	BaseView::save_settings(settings);

	viewhelper::save_signal(voltage_signal_, settings, "v_");
	viewhelper::save_signal(current_signal_, settings, "i_");
}

void PowerPanelView::restore_settings(QSettings &settings)
{
	BaseView::restore_settings(settings);

	auto v_signal = viewhelper::restore_signal(session_, settings, "v_");
	auto i_signal = viewhelper::restore_signal(session_, settings, "i_");
	if (v_signal && i_signal)
		set_signals(v_signal, i_signal);
}

void PowerPanelView::reset_displays()
{
	voltage_display_->reset_value();
	voltage_min_display_->reset_value();
	voltage_max_display_->reset_value();

	current_display_->reset_value();
	current_min_display_->reset_value();
	current_max_display_->reset_value();

	resistance_display_->reset_value();
	resistance_min_display_->reset_value();
	resistance_max_display_->reset_value();

	power_display_->reset_value();
	power_min_display_->reset_value();
	power_max_display_->reset_value();

	amp_hour_display_->reset_value();
	watt_hour_display_->reset_value();
}

void PowerPanelView::init_timer()
{
	start_time_ = QDateTime::currentMSecsSinceEpoch();
	last_time_ = start_time_;

	voltage_min_ = std::numeric_limits<double>::max();
	voltage_max_ = std::numeric_limits<double>::lowest();
	current_min_ = std::numeric_limits<double>::max();
	current_max_ = std::numeric_limits<double>::lowest();
	resistance_min_ = std::numeric_limits<double>::max();
	resistance_max_ = std::numeric_limits<double>::lowest();
	power_min_ = std::numeric_limits<double>::max();
	power_max_ = std::numeric_limits<double>::lowest();
	actual_amp_hours_ = 0;
	actual_watt_hours_ = 0;

	connect(timer_, &QTimer::timeout, this, &PowerPanelView::on_update);
	timer_->start(250);
}

void PowerPanelView::stop_timer()
{
	if (!timer_->isActive())
		return;

	timer_->stop();
	disconnect(timer_, &QTimer::timeout, this, &PowerPanelView::on_update);

	reset_displays();
}

void PowerPanelView::on_update()
{
	if (voltage_signal_->sample_count() == 0 ||
			current_signal_->sample_count() == 0)
		return;

	qint64 now = QDateTime::currentMSecsSinceEpoch();
	double elapsed_time = (double)(now - last_time_) / (double)3600000; // / 1h
	last_time_ = now;

	double voltage = 0.;
	if (voltage_signal_) {
		voltage = voltage_signal_->last_value();
		if (voltage_min_ > voltage)
			voltage_min_ = voltage;
		if (voltage_max_ < voltage)
			voltage_max_ = voltage;
	}

	double current = 0.;
	if (current_signal_) {
		current = current_signal_->last_value();
		if (current_min_ > current)
			current_min_ = current;
		if (current_max_ < current)
			current_max_ = current;
	}

	double resistance = current == 0. ?
		std::numeric_limits<double>::max() : voltage / current;
	if (resistance_min_ > resistance)
		resistance_min_ = resistance;
	if (resistance_max_ < resistance)
		resistance_max_ = resistance;

	double power = voltage * current;
	if (power_min_ > power)
		power_min_ = power;
	if (power_max_ < power)
		power_max_ = power;

	actual_amp_hours_ = actual_amp_hours_ + (current * elapsed_time);
	actual_watt_hours_ = actual_watt_hours_ + (power * elapsed_time);

	voltage_display_->set_value(voltage);
	voltage_min_display_->set_value(voltage_min_);
	voltage_max_display_->set_value(voltage_max_);

	current_display_->set_value(current);
	current_min_display_->set_value(current_min_);
	current_max_display_->set_value(current_max_);

	resistance_display_->set_value(resistance);
	resistance_min_display_->set_value(resistance_min_);
	resistance_max_display_->set_value(resistance_max_);

	power_display_->set_value(power);
	power_min_display_->set_value(power_min_);
	power_max_display_->set_value(power_max_);

	amp_hour_display_->set_value(actual_amp_hours_);
	watt_hour_display_->set_value(actual_watt_hours_);
}

void PowerPanelView::on_action_reset_displays_triggered()
{
	stop_timer();
	init_timer();
}

void PowerPanelView::on_digits_changed()
{
	voltage_display_->set_digits(
		voltage_signal_->digits(), voltage_signal_->decimal_places());
	voltage_min_display_->set_digits(
		voltage_signal_->digits(), voltage_signal_->decimal_places());
	voltage_max_display_->set_digits(
		voltage_signal_->digits(), voltage_signal_->decimal_places());

	current_display_->set_digits(
		current_signal_->digits(), current_signal_->decimal_places());
	current_min_display_->set_digits(
		current_signal_->digits(), current_signal_->decimal_places());
	current_max_display_->set_digits(
		current_signal_->digits(), current_signal_->decimal_places());

	// Use the smaller digits count to save space.
	int digits;
	if (voltage_signal_->digits() > current_signal_->digits())
		digits = current_signal_->digits();
	else
		digits = voltage_signal_->digits();
	// Use the smaller decimal_places count to save space.
	int decimal_places;
	if (voltage_signal_->decimal_places() > current_signal_->decimal_places())
		decimal_places = current_signal_->decimal_places();
	else
		decimal_places = voltage_signal_->decimal_places();

	resistance_display_->set_digits(digits, decimal_places);
	resistance_min_display_->set_digits(digits, decimal_places);
	resistance_max_display_->set_digits(digits, decimal_places);
	power_display_->set_digits(digits, decimal_places);
	power_min_display_->set_digits(digits, decimal_places);
	power_max_display_->set_digits(digits, decimal_places);
	amp_hour_display_->set_digits(digits, decimal_places);
	watt_hour_display_->set_digits(digits, decimal_places);
}

} // namespace views
} // namespace ui
} // namespace sv
