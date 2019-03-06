/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <set>

#include <QApplication>
#include <QDateTime>
#include <QVBoxLayout>

#include "powerpanelview.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/datautil.hpp"
#include "src/ui/widgets/lcddisplay.hpp"

using std::set;
using sv::data::QuantityFlag;

namespace sv {
namespace ui {
namespace views {

PowerPanelView::PowerPanelView(Session &session,
		shared_ptr<sv::data::AnalogSignal> voltage_signal,
		shared_ptr<sv::data::AnalogSignal> current_signal,
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
	QGridLayout *panel_layout = new QGridLayout();

	QString voltage_unit_suffix("");
	set<QuantityFlag> voltage_qfs = voltage_signal_->quantity_flags();
	if (voltage_qfs.count(QuantityFlag::AC)) {
		//voltage_unit_suffix = QString::fromUtf8("\u23E6");
		voltage_unit_suffix = sv::data::datautil::format_quantity_flag(
			QuantityFlag::AC);
		voltage_qfs.erase(QuantityFlag::AC);
	}
	else if (voltage_qfs.count(QuantityFlag::DC)) {
		//voltage_unit_suffix = QString::fromUtf8("\u2393");
		voltage_unit_suffix = sv::data::datautil::format_quantity_flag(
			QuantityFlag::DC);
		voltage_qfs.erase(QuantityFlag::DC);
	}
	set<QuantityFlag> voltage_qfs_min = voltage_qfs;
	voltage_qfs_min.insert(QuantityFlag::Min);
	set<QuantityFlag> voltage_qfs_max = voltage_qfs;
	voltage_qfs_max.insert(QuantityFlag::Max);

	voltage_display_ = new widgets::LcdDisplay(
		voltage_signal_->digits(), voltage_signal_->decimal_places(), false,
		voltage_signal_->unit_name(), voltage_unit_suffix,
		sv::data::datautil::format_quantity_flags(voltage_qfs, "\n"), false);
	voltage_min_display_ = new widgets::LcdDisplay(
		voltage_signal_->digits(), voltage_signal_->decimal_places(), false,
		voltage_signal_->unit_name(), voltage_unit_suffix,
		sv::data::datautil::format_quantity_flags(voltage_qfs_min, "\n"), true);
	voltage_max_display_ = new widgets::LcdDisplay(
		voltage_signal_->digits(), voltage_signal_->decimal_places(), false,
		voltage_signal_->unit_name(), voltage_unit_suffix,
		sv::data::datautil::format_quantity_flags(voltage_qfs_max, "\n"), true);

	QString current_unit_suffix("");
	set<QuantityFlag> current_qfs = current_signal_->quantity_flags();
	if (current_qfs.count(QuantityFlag::AC)) {
		current_unit_suffix = sv::data::datautil::format_quantity_flag(
			QuantityFlag::AC);
		current_qfs.erase(QuantityFlag::AC);
	}
	else if (current_qfs.count(QuantityFlag::DC)) {
		current_unit_suffix = sv::data::datautil::format_quantity_flag(
			QuantityFlag::DC);
		current_qfs.erase(QuantityFlag::DC);
	}
	set<QuantityFlag> current_qfs_min = current_qfs;
	current_qfs_min.insert(QuantityFlag::Min);
	set<QuantityFlag> current_qfs_max = current_qfs;
	current_qfs_max.insert(QuantityFlag::Max);

	current_display_ = new widgets::LcdDisplay(
		current_signal_->digits(), current_signal_->decimal_places(), false,
		current_signal_->unit_name(), current_unit_suffix,
		sv::data::datautil::format_quantity_flags(current_qfs, "\n"), false);
	current_min_display_ = new widgets::LcdDisplay(
		current_signal_->digits(), current_signal_->decimal_places(), false,
		current_signal_->unit_name(), current_unit_suffix,
		sv::data::datautil::format_quantity_flags(current_qfs_min, "\n"), true);
	current_max_display_ = new widgets::LcdDisplay(
		current_signal_->digits(), current_signal_->decimal_places(), false,
		current_signal_->unit_name(), current_unit_suffix,
		sv::data::datautil::format_quantity_flags(current_qfs_max, "\n"), true);

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

	resistance_display_ = new widgets::LcdDisplay(
		digits, decimal_places, true,
		sv::data::datautil::format_unit(data::Unit::Ohm), "", "", false);
	resistance_min_display_ = new widgets::LcdDisplay(
		digits, decimal_places, true,
		sv::data::datautil::format_unit(data::Unit::Ohm), "",
		sv::data::datautil::format_quantity_flag(QuantityFlag::Min),
		true);
	resistance_max_display_ = new widgets::LcdDisplay(
		digits, decimal_places, true,
		sv::data::datautil::format_unit(data::Unit::Ohm), "",
		sv::data::datautil::format_quantity_flag(QuantityFlag::Max),
		true);

	power_display_ = new widgets::LcdDisplay(
		digits, decimal_places, true,
		sv::data::datautil::format_unit(data::Unit::Watt), "", "", false);
	power_min_display_ = new widgets::LcdDisplay(
		digits, decimal_places, true,
		sv::data::datautil::format_unit(data::Unit::Watt), "",
		sv::data::datautil::format_quantity_flag(QuantityFlag::Min),
		true);
	power_max_display_ = new widgets::LcdDisplay(
		digits, decimal_places, true,
		sv::data::datautil::format_unit(data::Unit::Watt), "",
		sv::data::datautil::format_quantity_flag(QuantityFlag::Max),
		true);

	amp_hour_display_ = new widgets::LcdDisplay(
		digits, decimal_places, true,
		sv::data::datautil::format_unit(data::Unit::AmpereHour), "", "", false);
	watt_hour_display_ = new widgets::LcdDisplay(
		digits, decimal_places, true,
		sv::data::datautil::format_unit(data::Unit::WattHour), "", "", false);

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

	this->central_widget_->setLayout(layout);
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
	connect(voltage_signal_.get(), SIGNAL(digits_changed(const int, const int)),
		voltage_display_, SLOT(set_digits(const int, const int)));
	connect(voltage_signal_.get(), SIGNAL(digits_changed(const int, const int)),
		voltage_min_display_, SLOT(set_digits(const int, const int)));
	connect(voltage_signal_.get(), SIGNAL(digits_changed(const int, const int)),
		voltage_max_display_, SLOT(set_digits(const int, const int)));

	connect(current_signal_.get(), SIGNAL(digits_changed(const int, const int)),
		current_display_, SLOT(set_digits(const int, const int)));
	connect(current_signal_.get(), SIGNAL(digits_changed(const int, const int)),
		current_min_display_, SLOT(set_digits(const int, const int)));
	connect(current_signal_.get(), SIGNAL(digits_changed(const int, const int)),
		current_max_display_, SLOT(set_digits(const int, const int)));

	// TODO: set_digits() for the other displays!
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

} // namespace views
} // namespace ui
} // namespace sv
