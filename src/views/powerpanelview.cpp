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
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "powerpanelview.hpp"
#include "src/session.hpp"
#include "src/data/analogdata.hpp"
#include "src/data/basesignal.hpp"
#include "src/widgets/lcddisplay.hpp"

namespace sv {
namespace views {

PowerPanelView::PowerPanelView(Session &session,
	shared_ptr<data::BaseSignal> voltage_signal,
	shared_ptr<data::BaseSignal> current_signal,
	QWidget *parent) :
		BaseView(session, parent),
	voltage_signal_(voltage_signal),
	current_signal_(current_signal)
{
	setup_ui();
	connect_signals();
	reset_displays();

	timer_ = new QTimer(this);
	init_timer();
}

PowerPanelView::~PowerPanelView()
{
	stop_timer();
}

void PowerPanelView::setup_ui()
{
	QVBoxLayout *getValuesVLayout = new QVBoxLayout(this);

	voltageDisplay = new widgets::LcdDisplay(5, "V", this);
	currentDisplay = new widgets::LcdDisplay(5, "A", this);
	resistanceDisplay = new widgets::LcdDisplay(
		5, QString::fromUtf8("\u2126"), this);
	powerDisplay = new widgets::LcdDisplay(5, "W", this);
	ampHourDisplay = new widgets::LcdDisplay(5, "Ah", this);
	wattHourDisplay = new widgets::LcdDisplay(5, "Wh", this);

	QHBoxLayout *upperHLayout = new QHBoxLayout(this);
	upperHLayout->addWidget(voltageDisplay);
	upperHLayout->addWidget(powerDisplay);
	upperHLayout->addWidget(ampHourDisplay);
	upperHLayout->addStretch(5);

	QHBoxLayout *lowerHLayout = new QHBoxLayout(this);
	lowerHLayout->addWidget(currentDisplay);
	lowerHLayout->addWidget(resistanceDisplay);
	lowerHLayout->addWidget(wattHourDisplay);
	lowerHLayout->addStretch(5);

	QHBoxLayout *buttonHLayout = new QHBoxLayout(this);
	resetButton = new QPushButton(this);
	resetButton->setText(
		QApplication::translate("SmuView", "Reset", Q_NULLPTR));
	buttonHLayout->addWidget(resetButton);
	buttonHLayout->addStretch(5);

	getValuesVLayout->addItem(upperHLayout);
	getValuesVLayout->addItem(lowerHLayout);
	getValuesVLayout->addItem(buttonHLayout);
	getValuesVLayout->addStretch(4);
}

void PowerPanelView::connect_signals()
{
	connect(resetButton, SIGNAL(clicked(bool)), this, SLOT(on_reset()));
}

void PowerPanelView::reset_displays()
{
	voltageDisplay->reset_value();
	currentDisplay->reset_value();
	resistanceDisplay->reset_value();
	powerDisplay->reset_value();
	ampHourDisplay->reset_value();
	wattHourDisplay->reset_value();
}

void PowerPanelView::init_timer()
{
	if (!voltage_signal_ && !current_signal_)
		return;

	start_time_ = QDateTime::currentMSecsSinceEpoch();
	last_time_ = start_time_;
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

void PowerPanelView::on_reset()
{
	stop_timer();
	init_timer();
}

void PowerPanelView::on_update()
{
	if (voltage_signal_->analog_data()->get_sample_count() == 0)
		return;

	qint64 now = QDateTime::currentMSecsSinceEpoch();
	double elapsed_time = (now - last_time_) / (double)3600000; // 1000 * 60 * 60 = 1h
	last_time_ = now;

	double voltage = 0;
	if (voltage_signal_ && voltage_signal_->analog_data())
		voltage = voltage_signal_->analog_data()->last_value();

	double current = 0;
	if (current_signal_ && current_signal_->analog_data())
		current = current_signal_->analog_data()->last_value();

	double resistance = current == 0 ?
		std::numeric_limits<double>::max() : voltage / current;
	double power = voltage * current;
	actual_amp_hours_ = actual_amp_hours_ + (current * elapsed_time);
	actual_watt_hours_ = actual_watt_hours_ + (power * elapsed_time);

	voltageDisplay->set_value(voltage);
	currentDisplay->set_value(current);
	resistanceDisplay->set_value(resistance);
	powerDisplay->set_value(power);
	ampHourDisplay->set_value(actual_amp_hours_);
	wattHourDisplay->set_value(actual_watt_hours_);
}

} // namespace views
} // namespace sv

