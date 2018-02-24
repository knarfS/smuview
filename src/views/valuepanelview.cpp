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
#include <memory>
#include <set>

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "valuepanelview.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/widgets/lcddisplay.hpp"

using std::dynamic_pointer_cast;
using std::set;

namespace sv {
namespace views {

ValuePanelView::ValuePanelView(const Session &session,
		shared_ptr<channels::BaseChannel> channel,
		QWidget *parent) :
	BaseView(session, parent),
	channel_(channel),
	unit_(""),
	unit_suffix_(""),
	value_min_(std::numeric_limits<double>::max()),
	value_max_(std::numeric_limits<double>::lowest()),
	action_reset_display_(new QAction(this))
{
	assert(channel_);

	signal_ = dynamic_pointer_cast<data::AnalogSignal>(
		channel_->actual_signal());

	digits_ = 7; // TODO
	decimal_places_ = -1; // TODO

	if (signal_)
		setup_unit();

	setup_ui();
	setup_toolbar();
	connect_signals_displays();
	reset_display();

	// Signal (aka Quantity + Flags + Unit) can change, e.g. DMM signals
	connect(channel_.get(), SIGNAL(signal_added(shared_ptr<data::BaseSignal>)),
		this, SLOT(on_signal_changed()));
	connect(channel_.get(), SIGNAL(signal_changed(shared_ptr<data::BaseSignal>)),
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
	unit_(""),
	unit_suffix_(""),
	value_min_(std::numeric_limits<double>::max()),
	value_max_(std::numeric_limits<double>::lowest()),
	action_reset_display_(new QAction(this))
{
	assert(signal_);

	digits_ = signal_->digits();
	decimal_places_ = signal_->decimal_places();

	setup_unit();
	setup_ui();
	setup_toolbar();
	connect_signals_displays();
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

void ValuePanelView::setup_unit()
{
	unit_ = signal_->unit_name();
	quantity_flags_ = signal_->quantity_flags();

	if (quantity_flags_.count(data::QuantityFlag::AC)) {
		//unit_suffix_ = QString::fromUtf8("\u23E6");
		unit_suffix_ = data::quantityutil::format_quantity_flag(
			data::QuantityFlag::AC);
		quantity_flags_.erase(data::QuantityFlag::AC);
	}
	else if (quantity_flags_.count(data::QuantityFlag::DC)) {
		//unit_suffix_ = QString::fromUtf8("\u2393");
		unit_suffix_ = data::quantityutil::format_quantity_flag(
			data::QuantityFlag::DC);
		quantity_flags_.erase(data::QuantityFlag::DC);
	}

	quantity_flags_min_ = quantity_flags_;
	quantity_flags_min_.insert(data::QuantityFlag::Min);
	quantity_flags_max_ = quantity_flags_;
	quantity_flags_max_.insert(data::QuantityFlag::Max);
}

void ValuePanelView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	QGridLayout *panelLayout = new QGridLayout();

	valueDisplay = new widgets::LcdDisplay(
		digits_, decimal_places_, true, unit_, unit_suffix_,
		data::quantityutil::format_quantity_flags(quantity_flags_, "\n"),
		false);
	valueMinDisplay = new widgets::LcdDisplay(
		digits_, decimal_places_, true, unit_, unit_suffix_,
		data::quantityutil::format_quantity_flags(quantity_flags_min_, "\n"),
		true);
	valueMaxDisplay = new widgets::LcdDisplay(
		digits_, decimal_places_, true, unit_, unit_suffix_,
		data::quantityutil::format_quantity_flags(quantity_flags_max_, "\n"),
		true);

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

void ValuePanelView::connect_signals_displays()
{
	if (signal_) {
		//connect(signal_.get(), SIGNAL(unit_changed(QString)),
		//	valueDisplay, SLOT(set_unit(const String)));
		connect(signal_.get(), SIGNAL(digits_changed(int)),
			valueDisplay, SLOT(set_digits(const int)));
		connect(signal_.get(), SIGNAL(decimal_places_changed(int)),
			valueDisplay, SLOT(set_decimal_places(const int)));

		connect(signal_.get(), SIGNAL(digits_changed(int)),
			valueMinDisplay, SLOT(set_digits(const int)));
		connect(signal_.get(), SIGNAL(decimal_places_changed(int)),
			valueMinDisplay, SLOT(set_decimal_places(const int)));

		connect(signal_.get(), SIGNAL(digits_changed(int)),
			valueMaxDisplay, SLOT(set_digits(const int)));
		connect(signal_.get(), SIGNAL(decimal_places_changed(int)),
			valueMaxDisplay, SLOT(set_decimal_places(const int)));
	}
}

void ValuePanelView::disconnect_signals_displays()
{
	if (signal_) {
		//disconnect(signal_.get(), SIGNAL(unit_changed(QString)),
		//	valueDisplay, SLOT(set_unit(QString)));
		disconnect(signal_.get(), SIGNAL(digits_changed(int)),
			valueDisplay, SLOT(set_digits(const int)));
		disconnect(signal_.get(), SIGNAL(decimal_places_changed(int)),
			valueDisplay, SLOT(set_decimal_places(const int)));

		disconnect(signal_.get(), SIGNAL(digits_changed(int)),
			valueMinDisplay, SLOT(set_digits(const int)));
		disconnect(signal_.get(), SIGNAL(decimal_places_changed(int)),
			valueMinDisplay, SLOT(set_decimal_places(const int)));

		disconnect(signal_.get(), SIGNAL(digits_changed(int)),
			valueMaxDisplay, SLOT(set_digits(const int)));
		disconnect(signal_.get(), SIGNAL(decimal_places_changed(int)),
			valueMaxDisplay, SLOT(set_decimal_places(const int)));
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
	// When channel_ is not set, we have a fixed signal_ and nothing will change
	if (!channel_)
		return;

	disconnect_signals_displays();

	signal_ = dynamic_pointer_cast<data::AnalogSignal>(
		channel_->actual_signal());

	setup_unit();
	digits_ = signal_->digits();
	decimal_places_ = signal_->decimal_places();

	valueDisplay->set_unit(unit_);
	valueDisplay->set_unit_suffix(unit_suffix_);
	valueDisplay->set_extra_text(
		data::quantityutil::format_quantity_flags(quantity_flags_, "\n"));
	valueDisplay->set_digits(digits_);
	valueDisplay->set_decimal_places(decimal_places_);

	valueMinDisplay->set_unit(unit_);
	valueMinDisplay->set_unit_suffix(unit_suffix_);
	valueMinDisplay->set_extra_text(
		data::quantityutil::format_quantity_flags(quantity_flags_min_, "\n"));
	valueMinDisplay->set_digits(digits_);
	valueMinDisplay->set_decimal_places(decimal_places_);

	valueMaxDisplay->set_unit(unit_);
	valueMaxDisplay->set_unit_suffix(unit_suffix_);
	valueMaxDisplay->set_extra_text(
		data::quantityutil::format_quantity_flags(quantity_flags_max_, "\n"));
	valueMaxDisplay->set_digits(digits_);
	valueMaxDisplay->set_decimal_places(decimal_places_);

	this->parentWidget()->setWindowTitle(this->title());

	connect_signals_displays();
}

void ValuePanelView::on_action_reset_display_triggered()
{
	stop_timer();
	init_timer();
}

} // namespace views
} // namespace sv

