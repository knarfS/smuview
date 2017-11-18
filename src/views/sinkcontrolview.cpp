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

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "sinkcontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/widgets/controlbutton.hpp"
#include "src/widgets/valuecontrol.hpp"

namespace sv {
namespace views {

SinkControlView::SinkControlView(Session &session,
	shared_ptr<devices::HardwareDevice> device, QWidget *parent) :
		BaseView(session, parent),
	device_(device)
{
	setup_ui();
	connect_signals();
	init_values();
}

void SinkControlView::setup_ui()
{
	QHBoxLayout *setHLayout = new QHBoxLayout(this);
	QVBoxLayout *setValuesVLayout = new QVBoxLayout(this);

	double min;
	double max;
	double step;
	device_->list_current_limit(min, max, step);
	setValueControl = new widgets::ValueControl(5, "A", min, max, step, this);
	setValuesVLayout->addWidget(setValueControl);
	setValuesVLayout->addStretch(5);
	setHLayout->addItem(setValuesVLayout);

	// Enable button
	QVBoxLayout *setOptionsVLayout = new QVBoxLayout(this);
	setEnableButton = new widgets::ControlButton(
		device_->is_enable_getable(), device_->is_enable_setable(), this);
	setOptionsVLayout->addWidget(setEnableButton);
	setHLayout->addItem(setOptionsVLayout);
}

void SinkControlView::connect_signals()
{
	// Control elements -> Device
	connect(setValueControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_value_changed(const double)));
	connect(setEnableButton, SIGNAL(state_changed(const bool)),
		this, SLOT(on_enabled_changed(const bool)));

	// Device -> Control elements
	connect(device_.get(), SIGNAL(current_limit_changed(const double)),
		setValueControl, SLOT(change_value(const double)));
	connect(device_.get(), SIGNAL(enabled_changed(const bool)),
		setEnableButton, SLOT(on_state_changed(const bool)));
}

void SinkControlView::init_values()
{
	setValueControl->on_value_changed(device_->get_current_limit());
	setEnableButton->on_state_changed(device_->get_enabled());
}

void SinkControlView::on_value_changed(const double value)
{
	device_->set_current_limit(value);
}

void SinkControlView::on_enabled_changed(const bool enabled)
{
	device_->set_enable(enabled);
}

} // namespace views
} // namespace sv

