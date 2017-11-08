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

#include <QHBoxLayout>
#include <QGroupBox>

#include "sourceview.hpp"
#include "src/widgets/controlbutton.hpp"
#include "src/widgets/powerpanel.hpp"
#include "src/widgets/valuecontrol.hpp"

namespace sv {
namespace views {

SourceView::SourceView(shared_ptr<devices::HardwareDevice> device,
		QWidget *parent) :
	QWidget(parent),
	device_(device)
{
	setup_ui();

	init_values();

	// Control elements -> device
	connect(setVoltageControl, SIGNAL(value_changed(double)),
		this, SLOT(on_voltage_changed(double)));
	connect(setCurrentControl, SIGNAL(value_changed(double)),
		this, SLOT(on_current_changed(double)));
	connect(setEnableButton, SIGNAL(state_changed(bool)),
		this, SLOT(on_enabled_changed(const bool)));

	// Device -> control elements
	connect(device_.get(), SIGNAL(voltage_target_changed(const double)),
		setVoltageControl, SLOT(change_value(const double)));
	connect(device_.get(), SIGNAL(current_limit_changed(const double)),
		setCurrentControl, SLOT(change_value(const double)));
	connect(device_.get(), SIGNAL(enabled_changed(const bool)),
		setEnableButton, SLOT(on_state_changed(const bool)));
}


void SourceView::init_values()
{
	setVoltageControl->on_value_changed(device_->get_voltage_target());
	setCurrentControl->on_value_changed(device_->get_current_limit());
	setEnableButton->on_state_changed(device_->get_enabled());
}

void SourceView::setup_ui()
{
	QHBoxLayout *mainLayout = new QHBoxLayout(this);

	// Group Box for Setting
	QGroupBox *setValuesGroupBox = new QGroupBox(this);
	setValuesGroupBox->setTitle("Set Values");

	QVBoxLayout *setValuesLayout = new QVBoxLayout(this);

	setEnableButton = new widgets::ControlButton(
		device_->is_enable_getable(), device_->is_enable_setable(), this);
	setValuesLayout->addWidget(setEnableButton);

	double min;
	double max;
	double step;
	device_->list_voltage_target(min, max, step);
	setVoltageControl = new widgets::ValueControl(3, "V", min, max, step, this);
	setValuesLayout->addWidget(setVoltageControl);

	device_->list_current_limit(min, max, step);
	setCurrentControl = new widgets::ValueControl(3, "A", min, max, step, this);
	setValuesLayout->addWidget(setCurrentControl);

	setValuesGroupBox->setLayout(setValuesLayout);

	// Group Box for Values
	QGroupBox *getValuesGroupBox = new QGroupBox(this);
	getValuesGroupBox->setTitle("Actual Values");
	QVBoxLayout *getValuesVLayout = new QVBoxLayout(this);

	// Power panel
	powerPanel = new widgets::PowerPanel(
		device_->voltage_signal(), device_->current_signal(), this);
	getValuesVLayout->addWidget(powerPanel);
	getValuesVLayout->addStretch(5);

	getValuesGroupBox->setLayout(getValuesVLayout);

	mainLayout->addWidget(setValuesGroupBox);
	mainLayout->addWidget(getValuesGroupBox);
}

void SourceView::on_voltage_changed(double value)
{
	device_->set_voltage_target(value);
}

void SourceView::on_current_changed(double value)
{
	device_->set_current_limit(value);
}

} // namespace views
} // namespace sv
