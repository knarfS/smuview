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
	value_signal_(value_signal),
	value_min_(std::numeric_limits<double>::max()),
	value_max_(std::numeric_limits<double>::lowest())
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
	// TODO: This is good enough for 7.5 digit multimeters, but should really
	// depend on the digits submitted by the analog packet.
	digits_ = 8;

	QVBoxLayout *layout = new QVBoxLayout();

	QGridLayout *panelLayout = new QGridLayout();

	valueDisplay = new widgets::LcdDisplay(digits_,
		value_signal_->analog_data()->unit(), "", false);
	valueMinDisplay = new widgets::LcdDisplay(digits_,
		value_signal_->analog_data()->unit(), "min", true);
	valueMaxDisplay = new widgets::LcdDisplay(digits_,
		value_signal_->analog_data()->unit(), "max", true);

	panelLayout->addWidget(valueDisplay, 0, 0, 1, 2, Qt::AlignHCenter);
	panelLayout->addWidget(valueMinDisplay, 1, 0, 1, 1, Qt::AlignHCenter);
	panelLayout->addWidget(valueMaxDisplay, 1, 1, 1, 1, Qt::AlignHCenter);
	layout->addLayout(panelLayout);

	resetButton = new QPushButton();
	resetButton->setText(tr("Reset"));
	layout->addWidget(resetButton);

	layout->addStretch(4);

	this->setLayout(layout);
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

	value_min_ = std::numeric_limits<double>::max();
	value_max_ = std::numeric_limits<double>::lowest();

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
	if (value_signal_ && value_signal_->analog_data()) {
		value = value_signal_->analog_data()->last_value();
		if (value_min_ > value)
			value_min_ = value;
		if (value_max_ < value)
			value_max_ = value;
	}

	valueDisplay->set_value(value);
	valueMinDisplay->set_value(value_min_);
	valueMaxDisplay->set_value(value_max_);
}

void ValuePanelView::on_quantity_changed(QString quantity)
{
	(QString)quantity;
	//valueDisplay->set_quantity(quantity);
}

void ValuePanelView::on_unit_changed(QString unit)
{
	valueDisplay->set_unit(unit);
	valueMinDisplay->set_unit(unit);
	valueMaxDisplay->set_unit(unit);
}

} // namespace views
} // namespace sv

