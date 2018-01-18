/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QApplication>
#include <QDateTime>
#include <QVBoxLayout>

#include "powerpanelview.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/data/analogsignal.hpp"
#include "src/widgets/lcddisplay.hpp"

namespace sv {
namespace views {

PowerPanelView::PowerPanelView(const Session &session,
		shared_ptr<data::AnalogSignal> voltage_signal,
		shared_ptr<data::AnalogSignal> current_signal,
		QWidget *parent) :
	BaseView(session, parent),
	voltage_signal_(voltage_signal),
	current_signal_(current_signal),
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
	return tr("Power Panel"); // TODO: channel group name
}

void PowerPanelView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();
	QGridLayout *panelLayout = new QGridLayout();

	voltageDisplay = new widgets::LcdDisplay(
		voltage_signal_->digits(), voltage_signal_->decimal_places(), false,
		voltage_signal_->unit(), "", false);
	voltageMinDisplay = new widgets::LcdDisplay(
		voltage_signal_->digits(), voltage_signal_->decimal_places(), false,
		voltage_signal_->unit(), tr("min"), true);
	voltageMaxDisplay = new widgets::LcdDisplay(
		voltage_signal_->digits(), voltage_signal_->decimal_places(), false,
		voltage_signal_->unit(), tr("max"), true);

	currentDisplay = new widgets::LcdDisplay(
		current_signal_->digits(), current_signal_->decimal_places(), false,
		current_signal_->unit(), "", false);
	currentMinDisplay = new widgets::LcdDisplay(
		current_signal_->digits(), current_signal_->decimal_places(), false,
		current_signal_->unit(), tr("min"), true);
	currentMaxDisplay = new widgets::LcdDisplay(
		current_signal_->digits(), current_signal_->decimal_places(), false,
		current_signal_->unit(), tr("max"), true);

	int digits;
	if (voltage_signal_->digits() > current_signal_->digits())
		digits = voltage_signal_->digits();
	else
		digits = current_signal_->digits();
	int decimal_places;
	if (voltage_signal_->decimal_places() > current_signal_->decimal_places())
		decimal_places = voltage_signal_->decimal_places();
	else
		decimal_places = current_signal_->decimal_places();

	resistanceDisplay = new widgets::LcdDisplay(
		digits, decimal_places, true,
		util::format_sr_unit(sigrok::Unit::OHM), "", false);
	resistanceMinDisplay = new widgets::LcdDisplay(
		digits, decimal_places, true,
		util::format_sr_unit(sigrok::Unit::OHM), tr("min"), true);
	resistanceMaxDisplay = new widgets::LcdDisplay(
		digits, decimal_places, true,
		util::format_sr_unit(sigrok::Unit::OHM), tr("max"), true);

	powerDisplay = new widgets::LcdDisplay(
		digits, decimal_places, true,
		util::format_sr_unit(sigrok::Unit::WATT), "", false);
	powerMinDisplay = new widgets::LcdDisplay(
		digits, decimal_places, true,
		util::format_sr_unit(sigrok::Unit::WATT), tr("min"), true);
	powerMaxDisplay = new widgets::LcdDisplay(
		digits, decimal_places, true,
		util::format_sr_unit(sigrok::Unit::WATT), tr("max"), true);

	// TODO: sigrok::Unit::AMP_HOUR missing!
	ampHourDisplay = new widgets::LcdDisplay(
		digits, decimal_places, true,
		"Ah", "", false);
	wattHourDisplay = new widgets::LcdDisplay(
		digits, decimal_places, true,
		util::format_sr_unit(sigrok::Unit::WATT_HOUR), "", false);

	panelLayout->addWidget(voltageDisplay, 0, 0, 1, 2, Qt::AlignHCenter);
	panelLayout->addWidget(voltageMinDisplay, 1, 0, 1, 1, Qt::AlignHCenter);
	panelLayout->addWidget(voltageMaxDisplay, 1, 1, 1, 1, Qt::AlignHCenter);

	panelLayout->addWidget(currentDisplay, 2, 0, 1, 2, Qt::AlignHCenter);
	panelLayout->addWidget(currentMinDisplay, 3, 0, 1, 1, Qt::AlignHCenter);
	panelLayout->addWidget(currentMaxDisplay, 3, 1, 1, 1, Qt::AlignHCenter);

	panelLayout->addWidget(resistanceDisplay, 0, 2, 1, 2, Qt::AlignHCenter);
	panelLayout->addWidget(resistanceMinDisplay, 1, 2, 1, 1, Qt::AlignHCenter);
	panelLayout->addWidget(resistanceMaxDisplay, 1, 3, 1, 1, Qt::AlignHCenter);

	panelLayout->addWidget(powerDisplay, 2, 2, 1, 2, Qt::AlignHCenter);
	panelLayout->addWidget(powerMinDisplay, 3, 2, 1, 1, Qt::AlignHCenter);
	panelLayout->addWidget(powerMaxDisplay, 3, 3, 1, 1, Qt::AlignHCenter);

	panelLayout->addWidget(ampHourDisplay, 0, 4, 2, 1, Qt::AlignCenter);
	panelLayout->addWidget(wattHourDisplay, 2, 4, 2, 1, Qt::AlignCenter);

	layout->addLayout(panelLayout);

	this->centralWidget_->setLayout(layout);
}

void PowerPanelView::setup_toolbar()
{
	action_reset_displays_->setText(tr("Reset displays"));
	action_reset_displays_->setIcon(
		QIcon::fromTheme("view-refresh",
		QIcon(":/icons/view-refresh.png")));
	connect(action_reset_displays_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_reset_displays_triggered()));

	toolbar_ = new QToolBar("Power Panel Toolbar");
	toolbar_->addAction(action_reset_displays_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void PowerPanelView::connect_signals()
{
	connect(voltage_signal_.get(), SIGNAL(digits_changed(int)),
		voltageDisplay, SLOT(set_digits(int)));
	connect(voltage_signal_.get(), SIGNAL(digits_changed(int)),
		voltageMinDisplay, SLOT(set_digits(int)));
	connect(voltage_signal_.get(), SIGNAL(digits_changed(int)),
		voltageMaxDisplay, SLOT(set_digits(int)));

	connect(current_signal_.get(), SIGNAL(digits_changed(int)),
		currentDisplay, SLOT(set_digits(int)));
	connect(current_signal_.get(), SIGNAL(digits_changed(int)),
		currentMinDisplay, SLOT(set_digits(int)));
	connect(current_signal_.get(), SIGNAL(digits_changed(int)),
		currentMaxDisplay, SLOT(set_digits(int)));

	// TODO: set_digits() for the other displays!
}

void PowerPanelView::reset_displays()
{
	voltageDisplay->reset_value();
	voltageMinDisplay->reset_value();
	voltageMaxDisplay->reset_value();

	currentDisplay->reset_value();
	currentMinDisplay->reset_value();
	currentMaxDisplay->reset_value();

	resistanceDisplay->reset_value();
	resistanceMinDisplay->reset_value();
	resistanceMaxDisplay->reset_value();

	powerDisplay->reset_value();
	powerMinDisplay->reset_value();
	powerMaxDisplay->reset_value();

	ampHourDisplay->reset_value();
	wattHourDisplay->reset_value();
}

void PowerPanelView::init_timer()
{
	if (!voltage_signal_ && !current_signal_)
		return;

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

	connect(timer_, SIGNAL(timeout()), this, SLOT(on_update()));
	timer_->start(250);
}

void PowerPanelView::stop_timer()
{
	if (!timer_->isActive())
		return;

	timer_->stop();
	disconnect(timer_, SIGNAL(timeout()), this, SLOT(on_update()));

	reset_displays();
}

void PowerPanelView::on_update()
{
	if (voltage_signal_->get_sample_count() == 0)
		return;

	qint64 now = QDateTime::currentMSecsSinceEpoch();
	double elapsed_time = (now - last_time_) / (double)3600000; // 1000 * 60 * 60 = 1h
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

	voltageDisplay->set_value(voltage);
	voltageMinDisplay->set_value(voltage_min_);
	voltageMaxDisplay->set_value(voltage_max_);

	currentDisplay->set_value(current);
	currentMinDisplay->set_value(current_min_);
	currentMaxDisplay->set_value(current_max_);

	resistanceDisplay->set_value(resistance);
	resistanceMinDisplay->set_value(resistance_min_);
	resistanceMaxDisplay->set_value(resistance_max_);

	powerDisplay->set_value(power);
	powerMinDisplay->set_value(power_min_);
	powerMaxDisplay->set_value(power_max_);

	ampHourDisplay->set_value(actual_amp_hours_);
	wattHourDisplay->set_value(actual_watt_hours_);
}

void PowerPanelView::on_action_reset_displays_triggered()
{
	stop_timer();
	init_timer();
}

} // namespace views
} // namespace sv

