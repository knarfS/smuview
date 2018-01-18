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
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/widgets/lcddisplay.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace views {

ValuePanelView::ValuePanelView(const Session &session,
		shared_ptr<channels::BaseChannel> channel,
		QWidget *parent) :
	BaseView(session, parent),
	channel_(channel),
	value_min_(std::numeric_limits<double>::max()),
	value_max_(std::numeric_limits<double>::lowest()),
	action_reset_display_(new QAction(this))
{
	assert(channel_);

	signal_ = dynamic_pointer_cast<data::AnalogSignal>(
		channel_->actual_signal());

	unit_ = QString("");
	digits_ = 7; // TODO
	decimal_places_ = -1; // TODO
	if (signal_) {
		digits_ = signal_->digits();
		decimal_places_ = signal_->decimal_places();
		unit_ = signal_->unit();
	}

	setup_ui();
	setup_toolbar();
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
	value_max_(std::numeric_limits<double>::lowest()),
	action_reset_display_(new QAction(this))
{
	assert(signal_);

	digits_ = signal_->digits();
	decimal_places_ = signal_->decimal_places();
	unit_ = signal_->unit();

	setup_ui();
	setup_toolbar();
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
		title = title.append(" ").append(channel_->display_name());

	return title;
}

ValuePanelView::~ValuePanelView()
{
	stop_timer();
}

void ValuePanelView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	QGridLayout *panelLayout = new QGridLayout();

	valueDisplay = new widgets::LcdDisplay(
		digits_, decimal_places_, true, unit_, "", false);
	valueMinDisplay = new widgets::LcdDisplay(
		digits_, decimal_places_, true, unit_, "min", true);
	valueMaxDisplay = new widgets::LcdDisplay(
		digits_, decimal_places_, true, unit_, "max", true);

	panelLayout->addWidget(valueDisplay, 0, 0, 1, 2, Qt::AlignHCenter);
	panelLayout->addWidget(valueMinDisplay, 1, 0, 1, 1, Qt::AlignHCenter);
	panelLayout->addWidget(valueMaxDisplay, 1, 1, 1, 1, Qt::AlignHCenter);
	layout->addLayout(panelLayout);

	layout->addStretch(4);

	this->centralWidget_->setLayout(layout);
}

void ValuePanelView::setup_toolbar()
{
	action_reset_display_->setText(tr("Reset display"));
	action_reset_display_->setIcon(
		QIcon::fromTheme("view-refresh",
		QIcon(":/icons/view-refresh.png")));
	connect(action_reset_display_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_reset_display_triggered()));

	toolbar_ = new QToolBar("Panel Toolbar");
	toolbar_->addAction(action_reset_display_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void ValuePanelView::connect_signals()
{
	if (signal_) {
		connect(signal_.get(), SIGNAL(digits_changed(int)),
			valueDisplay, SLOT(set_digits(int)));
		connect(signal_.get(), SIGNAL(digits_changed(int)),
			valueMinDisplay, SLOT(set_digits(int)));
		connect(signal_.get(), SIGNAL(digits_changed(int)),
			valueMaxDisplay, SLOT(set_digits(int)));
	}
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

	if (signal_) {
		disconnect(signal_.get(), SIGNAL(digits_changed(int)),
			valueDisplay, SLOT(set_digits(int)));
		disconnect(signal_.get(), SIGNAL(digits_changed(int)),
			valueMinDisplay, SLOT(set_digits(int)));
		disconnect(signal_.get(), SIGNAL(digits_changed(int)),
			valueMaxDisplay, SLOT(set_digits(int)));
	}

	signal_ = dynamic_pointer_cast<data::AnalogSignal>(
		channel_->actual_signal());

	digits_ = signal_->digits();
	decimal_places_ = signal_->decimal_places();
	unit_ = signal_->unit();
	valueDisplay->set_unit(unit_);
	valueMinDisplay->set_unit(unit_);
	valueMaxDisplay->set_unit(unit_);
	this->parentWidget()->setWindowTitle(this->title());

	connect(signal_.get(), SIGNAL(digits_changed(int)),
		valueDisplay, SLOT(set_digits(int)));
	connect(signal_.get(), SIGNAL(digits_changed(int)),
		valueMinDisplay, SLOT(set_digits(int)));
	connect(signal_.get(), SIGNAL(digits_changed(int)),
		valueMaxDisplay, SLOT(set_digits(int)));
}

void ValuePanelView::on_action_reset_display_triggered()
{
	stop_timer();
	init_timer();
}

} // namespace views
} // namespace sv

