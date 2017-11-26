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

#include "sourcecontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/widgets/controlbutton.hpp"
#include "src/widgets/valuecontrol.hpp"

namespace sv {
namespace views {

SourceControlView::SourceControlView(Session &session,
	shared_ptr<devices::HardwareDevice> device, QWidget *parent) :
		BaseView(session, parent),
	device_(device)
{
	setup_ui();
	connect_signals();
	init_values();
}

void SourceControlView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	setEnableButton = new widgets::ControlButton(
		device_->is_enable_getable(), device_->is_enable_setable());
	layout->addWidget(setEnableButton);

	double min;
	double max;
	double step;
	device_->list_voltage_target(min, max, step);
	setVoltageControl = new widgets::ValueControl(3, "V", min, max, step);
	layout->addWidget(setVoltageControl);

	device_->list_current_limit(min, max, step);
	setCurrentControl = new widgets::ValueControl(3, "A", min, max, step);
	layout->addWidget(setCurrentControl);

	this->setLayout(layout);
}

void SourceControlView::connect_signals()
{
	// Control elements -> Device
	connect(setVoltageControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_voltage_changed(const double)));
	connect(setCurrentControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_current_changed(const double)));
	connect(setEnableButton, SIGNAL(state_changed(const bool)),
		this, SLOT(on_enabled_changed(const bool)));

	// Device -> Control elements
	connect(device_.get(), SIGNAL(voltage_target_changed(const double)),
		setVoltageControl, SLOT(change_value(const double)));
	connect(device_.get(), SIGNAL(current_limit_changed(const double)),
		setCurrentControl, SLOT(change_value(const double)));
	connect(device_.get(), SIGNAL(enabled_changed(const bool)),
		setEnableButton, SLOT(on_state_changed(const bool)));
}

void SourceControlView::init_values()
{
	setVoltageControl->on_value_changed(device_->get_voltage_target());
	setCurrentControl->on_value_changed(device_->get_current_limit());
	setEnableButton->on_state_changed(device_->get_enabled());
}

void SourceControlView::on_voltage_changed(const double value)
{
	device_->set_voltage_target(value);
}

void SourceControlView::on_current_changed(const double value)
{
	device_->set_current_limit(value);
}

void SourceControlView::on_enabled_changed(const bool enabled)
{
	device_->set_enable(enabled);
}

} // namespace views
} // namespace sv

