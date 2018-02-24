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

#include "valuecontrol.hpp"
#include "src/util.hpp"

namespace sv {
namespace widgets {

ValueControl::ValueControl(
		const QString title, const int digits, const QString unit,
		const double min, const double max, const double steps,
		QWidget *parent) :
	QGroupBox(parent),
	title_(title),
	value_(0),
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
	this->setTitle(title_);

	QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	this->setSizePolicy(sizePolicy);

	QVBoxLayout *layout = new QVBoxLayout();

	lcdDisplay = new widgets::LcdDisplay(
		get_digits(), get_decimal_places(), false, unit_,
		QString(""), QString(""), false);
	lcdDisplay->set_value(0);
	layout->addWidget(lcdDisplay, 0, Qt::AlignRight);

	QHBoxLayout *getValuesHLayout = new QHBoxLayout();

	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setSuffix(QString(" %1").arg(unit_));
	doubleSpinBox->setDecimals(get_decimal_places());
	doubleSpinBox->setMinimum(min_);
	doubleSpinBox->setMaximum(max_);
	doubleSpinBox->setSingleStep(steps_);

	knob = new QwtKnob();
	knob->knobRect().setSize(QSize(100, 100));
	knob->setNumTurns(1);
	knob->setLowerBound(min_);
	knob->setUpperBound(max_);
	// setSingleSteps(uint), setPageSteps(uint)
	knob->setTotalSteps((max_ - min_) / steps_);
	//setScaleStepSize(double), setScaleMaxMajor(int), setScaleMaxMinor(int)

	getValuesHLayout->addWidget(doubleSpinBox);
	getValuesHLayout->addWidget(knob);
	layout->addItem(getValuesHLayout);

	this->setLayout(layout);
}

uint ValueControl::get_digits()
{
	int i = (int)floor(max_);
	return util::count_int_digits(i) + get_decimal_places();
}

uint ValueControl::get_decimal_places()
{
	int d = (int)ceil(1/steps_) - 1;
	uint cnt = util::count_int_digits(d);
	return cnt;
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
		this, SLOT(on_value_changed(const double)));
	connect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));
}

void ValueControl::on_value_changed(const double value)
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

	Q_EMIT value_changed(value);

	connect(knob, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));
	connect(doubleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(on_value_changed(const double)));
}

} // namespace widgets
} // namespace sv
