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

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QVBoxLayout>

#include "singlevaluepanel.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/analog.hpp"

namespace sv {
namespace widgets {

SingleValuePanel::SingleValuePanel(shared_ptr<data::BaseSignal> value_signal,
		QWidget *parent) :
	QWidget(parent),
	value_signal_(value_signal)
{
	setup_ui();
	reset_display();

	timer_ = new QTimer(this);
	init_timer();

	connect(resetButton, SIGNAL(clicked(bool)), this, SLOT(on_reset()));
}

SingleValuePanel::~SingleValuePanel()
{
	stop_timer();
}

void SingleValuePanel::setup_ui()
{
	QVBoxLayout *valueVLayout = new QVBoxLayout(this);

	valueDisplay = new widgets::LcdDisplay(5, "V", this);

	QHBoxLayout *valueHLayout = new QHBoxLayout(this);
	valueHLayout->addWidget(valueDisplay);
	valueHLayout->addStretch(5);

	QHBoxLayout *buttonHLayout = new QHBoxLayout(this);
	resetButton = new QPushButton(this);
	resetButton->setText(
		QApplication::translate("SmuView", "Reset", Q_NULLPTR));
	buttonHLayout->addWidget(resetButton);
	buttonHLayout->addStretch(5);

	valueVLayout->addItem(valueHLayout);
	valueVLayout->addItem(buttonHLayout);
	valueVLayout->addStretch(4);
}

void SingleValuePanel::reset_display()
{
	valueDisplay->reset_value();
}

void SingleValuePanel::init_timer()
{
	if (!value_signal_)
		return;

	start_time_ = QDateTime::currentMSecsSinceEpoch();
	last_time_ = start_time_;

    connect(timer_, SIGNAL(timeout()), this, SLOT(on_update()));
    timer_->start(250);
}

void SingleValuePanel::stop_timer()
{
	if (!timer_->isActive())
		return;

    timer_->stop();
    disconnect(timer_, SIGNAL(timeout()), this, SLOT(on_update()));

	reset_display();
}

void SingleValuePanel::on_reset()
{
	stop_timer();
	init_timer();
}

void SingleValuePanel::on_update()
{
	/*
	qint64 now = QDateTime::currentMSecsSinceEpoch();
	double elapsed_time = (now - last_time_) / (double)3600000; // 1000 * 60 * 60 = 1h
	last_time_ = now;
	*/
	last_time_ = QDateTime::currentMSecsSinceEpoch();

	double value = 0;
	if (value_signal_ && value_signal_->analog_data())
		value = value_signal_->analog_data()->last_value();

	valueDisplay->set_value(value);
}

} // namespace widgets
} // namespace sv

