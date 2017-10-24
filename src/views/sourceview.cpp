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
#include "src/widgets/lcddisplay.hpp"
#include "src/widgets/valuecontrol.hpp"

namespace sv {
namespace views {

SourceView::SourceView(shared_ptr<devices::HardwareDevice> device,
		QWidget *parent) :
	QWidget(parent),
	device_(device)
{
	setupUi();

	connect(setVoltageControl, SIGNAL(value_changed(double)),
		this, SLOT(on_voltage_changed(double)));
	connect(setCurrentControl, SIGNAL(value_changed(double)),
		this, SLOT(on_current_changed(double)));
}

void SourceView::setupUi()
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
	QHBoxLayout *getValuesLayout = new QHBoxLayout(this);
	voltageDisplay = new widgets::LcdDisplay(5, "V", this);
	currentDisplay = new widgets::LcdDisplay(5, "A", this);
	powerDisplay = new widgets::LcdDisplay(5, "W", this);
	getValuesLayout->addWidget(voltageDisplay);
	getValuesLayout->addWidget(currentDisplay);
	getValuesLayout->addWidget(powerDisplay);
	getValuesGroupBox->setLayout(getValuesLayout);

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
