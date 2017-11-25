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

#include "valuepanelview.hpp"
#include "src/session.hpp"
#include "src/data/analogdata.hpp"
#include "src/data/basesignal.hpp"
#include "src/widgets/lcddisplay.hpp"

namespace sv {
namespace views {

ValuePanelView::ValuePanelView(Session &session,
	shared_ptr<data::BaseSignal> value_signal,
	QWidget *parent) :
		BaseView(session, parent),
	value_signal_(value_signal)
{
	setup_ui();
	connect_signals();
	reset_display();

	timer_ = new QTimer(this);
	init_timer();
}

ValuePanelView::~ValuePanelView()
{
	stop_timer();
}

void ValuePanelView::setup_ui()
{
	QVBoxLayout *getValuesVLayout = new QVBoxLayout(this);

	//valueDisplay = new widgets::LcdDisplay(digits_, unit_, this);
	valueDisplay = new widgets::LcdDisplay(8, "mX", this);
	getValuesVLayout->addWidget(valueDisplay);

	resetButton = new QPushButton(this);
	resetButton->setText(
		QApplication::translate("SmuView", "Reset", Q_NULLPTR));
	getValuesVLayout->addWidget(resetButton);

	getValuesVLayout->addStretch(4);
}

void ValuePanelView::connect_signals()
{
	// Reset button
	connect(resetButton, SIGNAL(clicked(bool)), this, SLOT(on_reset()));

	// Signal stuff
	if (value_signal_->analog_data()) {
		connect(value_signal_->analog_data().get(),
			SIGNAL(quantity_changed(QString)),
			this, SLOT(on_quantity_changed(QString)));
		connect(value_signal_->analog_data().get(),
			SIGNAL(unit_changed(QString)),
			this, SLOT(on_unit_changed(QString)));
	}
}

void ValuePanelView::reset_display()
{
	valueDisplay->reset_value();
}

void ValuePanelView::init_timer()
{
	if (!value_signal_)
		return;

	connect(timer_, SIGNAL(timeout()), this, SLOT(on_update()));
	timer_->start(250);
}

void ValuePanelView::stop_timer()
{
	if (!timer_->isActive())
		return;

	timer_->stop();
	disconnect(timer_, SIGNAL(timeout()), this, SLOT(on_update()));

	reset_display();
}

void ValuePanelView::on_reset()
{
	stop_timer();
	init_timer();
}

void ValuePanelView::on_update()
{
	if (value_signal_->analog_data()->get_sample_count() == 0)
		return;

	double value = 0;
	if (value_signal_ && value_signal_->analog_data())
		value = value_signal_->analog_data()->last_value();

	valueDisplay->set_value(value);
}

void ValuePanelView::on_quantity_changed(QString quantity)
{
	(QString)quantity;
	//valueDisplay->set_quantity(quantity);
}

void ValuePanelView::on_unit_changed(QString unit)
{
	valueDisplay->set_unit(unit);
}

} // namespace views
} // namespace sv

