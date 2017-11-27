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
#include "src/widgets/controlbutton.hpp"

namespace sv {
namespace widgets {

OptionalValueControl::OptionalValueControl(const bool active,
		const bool is_readable, const bool is_setable,
		const uint digits, const QString unit,
		const double min, const double max, const double steps,
		QWidget *parent) :
	QWidget(parent),
	state_(active),
	value_(0),
	is_readable_(is_readable),
	is_setable_(is_setable),
	digits_(digits),
	unit_(unit),
	min_(min),
	max_(max),
	steps_(steps)
{
	setup_ui();

	connect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));
}

void OptionalValueControl::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	/*
	controlButton = new widgets::ControlButton(
		state_, is_readable_, is_setable_);
	layout->addWidget(controlButton);
	*/

	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setSuffix(QString(" %1").arg(unit_));
	doubleSpinBox->setDecimals(3);
	doubleSpinBox->setMinimum(min_);
	doubleSpinBox->setMaximum(max_);
	doubleSpinBox->setSingleStep(steps_);
	layout->addWidget(doubleSpinBox);

	this->setLayout(layout);
}

void OptionalValueControl::on_clicked()
{
	controlButton->on_clicked();
}

void OptionalValueControl::on_state_changed(const bool enabled)
{
	controlButton->on_state_changed(enabled);
}

void OptionalValueControl::change_value(const double value)
{
	if (value == value_)
		return;

	disconnect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));

	value_ = value;

	doubleSpinBox->setValue(value);

	connect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));
}

void OptionalValueControl::on_value_changed(const double value)
{
	if (value == value_)
		return;

	disconnect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));

	value_ = value;

	doubleSpinBox->setValue(value);

	Q_EMIT value_changed(value);

	connect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));
}

} // namespace widgets
} // namespace sv
