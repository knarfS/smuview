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

#include <QVBoxLayout>

#include "optionalvaluecontrol.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/widgets/controlbutton.hpp"

namespace sv {
namespace widgets {

OptionalValueControl::OptionalValueControl(
		bool (devices::HardwareDevice::*get_state_caller)() const,
		void (devices::HardwareDevice::*set_state_caller)(const bool),
		bool (devices::HardwareDevice::*is_state_getable_caller)() const,
		bool (devices::HardwareDevice::*is_state_setable_caller)() const,
		double (devices::HardwareDevice::*get_value_caller)() const,
		void (devices::HardwareDevice::*set_value_caller)(const double),
		bool (devices::HardwareDevice::*is_value_getable_caller)() const,
		bool (devices::HardwareDevice::*is_value_setable_caller)() const,
		shared_ptr<devices::HardwareDevice> device,
		const QString title, const QString unit,
		const double min, const double max, const double steps,
		QWidget *parent) :
	QGroupBox(parent),
	get_state_caller_(get_state_caller),
	set_state_caller_(set_state_caller),
	is_state_getable_caller_(is_state_getable_caller),
	is_state_setable_caller_(is_state_setable_caller),
	get_value_caller_(get_value_caller),
	set_value_caller_(set_value_caller),
	is_value_getable_caller_(is_value_getable_caller),
	is_value_setable_caller_(is_value_setable_caller),
	device_(device),
	title_(title),
	unit_(unit),
	min_(min),
	max_(max),
	steps_(steps)
{
	is_state_getable_ = (device_.get()->*is_state_getable_caller_)();
	is_state_setable_ = (device_.get()->*is_state_setable_caller_)();
	is_state_enabled_ = is_state_getable_ || is_state_setable_;
	if (is_state_getable_)
		state_ = (device_.get()->*get_state_caller_)();
	else
		state_ = false;

	is_value_getable_ = (device_.get()->*is_value_getable_caller_)();
	is_value_setable_ = (device_.get()->*is_value_setable_caller_)();
	if (is_value_getable_)
		value_ = (device_.get()->*get_value_caller_)();
	else
		value_ = 0;

	setup_ui();
	connect_signals();
}

void OptionalValueControl::setup_ui()
{
	this->setTitle(title_);

	QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	this->setSizePolicy(sizePolicy);

	QVBoxLayout *layout = new QVBoxLayout();

	controlButton = new widgets::ControlButton(
		get_state_caller_, set_state_caller_,
		is_state_getable_caller_, is_state_setable_caller_,
		device_);
	layout->addWidget(controlButton);

	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setSuffix(QString(" %1").arg(unit_));
	doubleSpinBox->setDecimals(3);
	doubleSpinBox->setMinimum(min_);
	doubleSpinBox->setMaximum(max_);
	doubleSpinBox->setSingleStep(steps_);
	doubleSpinBox->setDisabled(!is_value_setable_);
	layout->addWidget(doubleSpinBox);

	this->setLayout(layout);
}

void OptionalValueControl::connect_signals()
{
	if (is_value_setable_)
		connect(doubleSpinBox, SIGNAL(valueChanged(double)),
			this, SLOT(on_value_changed(const double)));
}

void OptionalValueControl::on_clicked()
{
	controlButton->on_clicked();
}

void OptionalValueControl::change_state(const bool state)
{
	if (state == state_)
		return;

	state_ = state;
	controlButton->on_state_changed(state);
}

void OptionalValueControl::on_state_changed(const bool state)
{
	if (is_state_setable_)
		controlButton->on_state_changed(state);
}

void OptionalValueControl::change_value(const double value)
{
	if (value == value_)
		return;

	if (is_value_setable_)
		disconnect(doubleSpinBox, SIGNAL(valueChanged(double)),
			this, SLOT(on_value_changed(const double)));

	value_ = value;

	doubleSpinBox->setValue(value);

	if (is_value_setable_)
		connect(doubleSpinBox, SIGNAL(valueChanged(double)),
			this, SLOT(on_value_changed(const double)));
}

void OptionalValueControl::on_value_changed(const double value)
{
	if (value == value_)
		return;

	if (is_value_setable_)
		disconnect(doubleSpinBox, SIGNAL(valueChanged(double)),
			this, SLOT(on_value_changed(const double)));

	value_ = value;

	doubleSpinBox->setValue(value);

	Q_EMIT value_changed(value);

	if (is_value_setable_)
		connect(doubleSpinBox, SIGNAL(valueChanged(double)),
			this, SLOT(on_value_changed(const double)));
}

} // namespace widgets
} // namespace sv
