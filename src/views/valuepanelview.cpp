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

#include <cassert>

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "valuepanelview.hpp"
#include "src/session.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/channel.hpp"
#include "src/widgets/lcddisplay.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace views {

ValuePanelView::ValuePanelView(const Session &session,
		shared_ptr<devices::Channel> channel,
		QWidget *parent) :
	BaseView(session, parent),
	channel_(channel),
	value_min_(std::numeric_limits<double>::max()),
	value_max_(std::numeric_limits<double>::lowest())
{
	assert(channel_);

	signal_ = dynamic_pointer_cast<data::AnalogSignal>(
		channel_->actual_signal());

	setup_ui();
	connect_signals();
	reset_display();

	// Signal (Quantity + Unit) can change, e.g. DMM signals
	connect(channel_.get(), SIGNAL(signal_changed()),
			this, SLOT(on_signal_changed()));

	timer_ = new QTimer(this);
	init_timer();
}


ValuePanelView::ValuePanelView(const Session& session,
		shared_ptr<data::AnalogSignal> signal,
		QWidget* parent) :
	BaseView(session, parent),
	channel_(nullptr),
	signal_(signal),
	value_min_(std::numeric_limits<double>::max()),
	value_max_(std::numeric_limits<double>::lowest())
{
	assert(signal_);

	setup_ui();
	connect_signals();
	reset_display();

	timer_ = new QTimer(this);
	init_timer();
}

QString ValuePanelView::title() const
{
	QString title;

	if (channel_)
		title = tr("Channel");
	else
		title = tr("Signal");

	if (signal_)
		title = title.append(" ").append(signal_->name());
	else if (channel_)
		title = title.append(" ").append(channel_->internal_name());

	return title;
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

	QString unit("");
	if (signal_)
		unit = signal_->unit();

	valueDisplay = new widgets::LcdDisplay(digits_, unit, "", false);
	valueMinDisplay = new widgets::LcdDisplay(digits_, unit, "min", true);
	valueMaxDisplay = new widgets::LcdDisplay(digits_, unit, "max", true);

	panelLayout->addWidget(valueDisplay, 0, 0, 1, 2, Qt::AlignHCenter);
	panelLayout->addWidget(valueMinDisplay, 1, 0, 1, 1, Qt::AlignHCenter);
	panelLayout->addWidget(valueMaxDisplay, 1, 1, 1, 1, Qt::AlignHCenter);
	layout->addLayout(panelLayout);

	resetButton = new QPushButton();
	resetButton->setText(tr("Reset"));
	layout->addWidget(resetButton);

	layout->addStretch(4);

	this->centralWidget_->setLayout(layout);
}

void ValuePanelView::connect_signals()
{
	// Reset button
	connect(resetButton, SIGNAL(clicked(bool)), this, SLOT(on_reset()));
}

void ValuePanelView::reset_display()
{
	valueDisplay->reset_value();
}

void ValuePanelView::init_timer()
{
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
	if (!signal_ || signal_->get_sample_count() == 0)
		return;

	double value = 0;
	if (signal_) {
		value = signal_->last_value();
		if (value_min_ > value)
			value_min_ = value;
		if (value_max_ < value)
			value_max_ = value;
	}

	valueDisplay->set_value(value);
	valueMinDisplay->set_value(value_min_);
	valueMaxDisplay->set_value(value_max_);
}

void ValuePanelView::on_signal_changed()
{
	if (!channel_)
		return;

	signal_ = dynamic_pointer_cast<data::AnalogSignal>(
		channel_->actual_signal());

	this->parentWidget()->setWindowTitle(this->title());
	valueDisplay->set_unit(signal_->unit());
	valueMinDisplay->set_unit(signal_->unit());
	valueMaxDisplay->set_unit(signal_->unit());
}

} // namespace views
} // namespace sv

