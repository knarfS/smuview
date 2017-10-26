/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDateTime>
#include <QDebug>
#include <QVBoxLayout>

#include "powerpanel.hpp"
#include "src/devices/hardwaredevice.hpp"

namespace sv {
namespace widgets {

PowerPanel::PowerPanel(shared_ptr<devices::HardwareDevice> device,
		QWidget *parent) :
	QWidget(parent),
	device_(device)
{
	setup_ui();
	init_timer();
}

PowerPanel::~PowerPanel()
{
	stop_timer();
}

void PowerPanel::setup_ui()
{
	QVBoxLayout *getValuesVLayout = new QVBoxLayout(this);

	voltageDisplay = new widgets::LcdDisplay(5, "V", this);
	currentDisplay = new widgets::LcdDisplay(5, "A", this);
	resistanceDisplay = new widgets::LcdDisplay(
		5, QString::fromUtf8("\u2126"), this);
	powerDisplay = new widgets::LcdDisplay(5, "W", this);
	ampHourDisplay = new widgets::LcdDisplay(5, "Ah", this);
	wattHourDisplay = new widgets::LcdDisplay(5, "Wh", this);

	QHBoxLayout *getUpperHLayout = new QHBoxLayout(this);
	getUpperHLayout->addWidget(voltageDisplay);
	getUpperHLayout->addWidget(powerDisplay);
	getUpperHLayout->addWidget(ampHourDisplay);
	getUpperHLayout->addStretch(5);

	QHBoxLayout *getLowerHLayout = new QHBoxLayout(this);
	getLowerHLayout->addWidget(currentDisplay);
	getLowerHLayout->addWidget(resistanceDisplay);
	getLowerHLayout->addWidget(wattHourDisplay);
	getLowerHLayout->addStretch(5);

	getValuesVLayout->addItem(getUpperHLayout);
	getValuesVLayout->addItem(getLowerHLayout);
	getValuesVLayout->addStretch(4);
}

void PowerPanel::init_timer()
{
	timer_ = new QTimer(this);

	start_time_ = QDateTime::currentMSecsSinceEpoch();
	last_time_ = start_time_;
	actual_amp_hours_ = 0;
	actual_watt_hours_ = 0;

    connect(timer_, SIGNAL(timeout()), this, SLOT(on_update()));
    timer_->start(250);
}

void PowerPanel::stop_timer()
{
    timer_->stop();
    disconnect(timer_, SIGNAL(timeout()), this, SLOT(on_update()));
}

void PowerPanel::reset()
{
	start_time_ = QDateTime::currentMSecsSinceEpoch();
	last_time_ = start_time_;
	actual_amp_hours_ = 0;
	actual_watt_hours_ = 0;
}

void PowerPanel::on_update()
{
	/*
	const int64_t last_sample = segment->get_sample_count() - 1;
	qWarning() << "on_data_received: last_sample" << last_sample;
	float *sample_block = new float[2];
	segment->get_samples(last_sample, last_sample+1, sample_block);

	double voltage = sample_block[1];
	double current = sample_block[0];
	*/

	//double voltage = device_->get_last_data_1();
	//double current = device_->get_last_data_2();
	double voltage = 3.3;
	double current = 0.5;
	double resistance = current == 0 ?
		std::numeric_limits<double>::max() : voltage / current;
	double power = voltage * current;

	qint64 now = QDateTime::currentMSecsSinceEpoch();
	double elapsed_time = (now - last_time_) / (double)3600000; // 1000 * 60 * 60 = 1h
	last_time_ = now;
	actual_amp_hours_ = actual_amp_hours_ + (current * elapsed_time);
	actual_watt_hours_ = actual_watt_hours_ + (power * elapsed_time);

	voltageDisplay->set_value(voltage);
	currentDisplay->set_value(current);
	resistanceDisplay->set_value(resistance);
	powerDisplay->set_value(power);
	ampHourDisplay->set_value(actual_amp_hours_);
	wattHourDisplay->set_value(actual_watt_hours_);
}

} // namespace widgets
} // namespace sv
