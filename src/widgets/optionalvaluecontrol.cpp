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

#include <QDebug>
#include <QVBoxLayout>

#include "optionalvaluecontrol.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/widgets/controlbutton.hpp"

namespace sv {
namespace widgets {

OptionalValueControl::OptionalValueControl(
		const bool is_state_getable, const bool is_state_setable,
		const bool is_value_getable, const bool is_value_setable,
		const QString title, const QString unit,
		const double min, const double max, const double steps,
		QWidget *parent) :
	QGroupBox(parent),
	is_state_getable_(is_state_getable),
	is_state_setable_(is_state_setable),
	is_value_getable_(is_value_getable),
	is_value_setable_(is_value_setable),
	title_(title),
	unit_(unit),
	min_(min),
	max_(max),
	steps_(steps)
{
	is_state_enabled_ = is_state_getable_ || is_state_setable_;
	if (!is_state_getable_)
		state_ = false;
	if (!is_value_getable_)
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
		is_state_getable_, is_state_setable_);
	controlButton->setSizePolicy(
		QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	layout->addWidget(controlButton);

	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setSizePolicy(
		QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
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
	if (is_state_setable_)
		connect(controlButton, SIGNAL(state_changed(const bool)),
			this, SLOT(on_state_changed(const bool)));

	if (is_value_setable_)
		connect(doubleSpinBox, SIGNAL(valueChanged(double)),
			this, SLOT(on_value_changed(const double)));
}

void OptionalValueControl::change_state(const bool state)
{
	if (state == state_)
		return;

	if (is_state_setable_)
		disconnect(controlButton, SIGNAL(state_changed(const bool)),
			this, SLOT(on_state_changed(const bool)));

	state_ = state;
	controlButton->change_state(state);

	if (is_state_setable_)
		connect(controlButton, SIGNAL(state_changed(const bool)),
			this, SLOT(on_state_changed(const bool)));
}

void OptionalValueControl::on_state_changed(const bool state)
{
	if (state == state_)
		return;

	change_state(state);
	Q_EMIT state_changed(state);
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

	change_value(value);
	Q_EMIT value_changed(value);
}

} // namespace widgets
} // namespace sv
