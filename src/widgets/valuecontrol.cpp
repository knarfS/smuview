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

#include "valuecontrol.hpp"

namespace sv {
namespace widgets {

ValueControl::ValueControl(const uint digits, const QString unit,
		const double min, const double max, const double steps,
		QWidget *parent) :
	QWidget(parent),
	digits_(digits),
	unit_(unit),
	min_(min),
	max_(max),
	steps_(steps)
{
	setup_ui();

	connect(knob, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));
	connect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));
}

void ValueControl::setup_ui()
{
	QVBoxLayout *getValuesVLayout = new QVBoxLayout(this);

	lcdDisplay = new widgets::LcdDisplay(digits_, unit_, this);
	getValuesVLayout->addWidget(lcdDisplay);

	QHBoxLayout *getValuesHLayout = new QHBoxLayout(this);

	doubleSpinBox = new QDoubleSpinBox(this);
	doubleSpinBox->setSuffix(QString(" %1").arg(unit_));
	doubleSpinBox->setDecimals(3);
	doubleSpinBox->setMinimum(min_);
	doubleSpinBox->setMaximum(max_);
	doubleSpinBox->setSingleStep(steps_);

	knob = new QwtKnob(this);
	knob->setNumTurns(1);
	knob->setLowerBound(min_);
	knob->setUpperBound(max_);
	// setSingleSteps(uint), setPageSteps(uint)
	knob->setTotalSteps((max_ - min_) / steps_);
	//setScaleStepSize(double), setScaleMaxMajor(int), setScaleMaxMinor(int)

	getValuesHLayout->addWidget(doubleSpinBox);
	getValuesHLayout->addWidget(knob);
	getValuesVLayout->addItem(getValuesHLayout);
}

void ValueControl::change_value(const double value)
{
	if (value == value_)
		return;

	disconnect(knob, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));
	disconnect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));

	value_ = value;

	lcdDisplay->set_value(value);
	doubleSpinBox->setValue(value);
	knob->setValue(value);

	connect(knob, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(double)));
	connect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(double)));
}

void ValueControl::on_value_changed(const double value)
{
	if (value == value_)
		return;
	if (value == value_)
		return;

	disconnect(knob, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));
	disconnect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));

	value_ = value;

	lcdDisplay->set_value(value);
	doubleSpinBox->setValue(value);
	knob->setValue(value);

	Q_EMIT value_changed(value);

	connect(knob, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(double)));
	connect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(double)));
}

} // namespace widgets
} // namespace sv
