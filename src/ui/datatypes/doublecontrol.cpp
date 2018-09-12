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

#include "doublecontrol.hpp"
#include "src/devices/properties/baseproperty.hpp"
#include "src/ui/datatypes/doubleknob.hpp"
#include "src/ui/datatypes/doublelcddisplay.hpp"
#include "src/ui/datatypes/doublespinbox.hpp"

namespace sv {
namespace ui {
namespace datatypes {

DoubleControl::DoubleControl(
		shared_ptr<devices::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QString title, QWidget *parent) :
	QGroupBox(parent),
	BaseWidget(property, auto_commit, auto_update),
	title_(title)
{
	setup_ui();
}

void DoubleControl::setup_ui()
{
	this->setTitle(title_);

	QSizePolicy size_policy(
		QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	size_policy.setHorizontalStretch(0);
	size_policy.setVerticalStretch(0);
	this->setSizePolicy(size_policy);

	QVBoxLayout *layout = new QVBoxLayout();

	lcd_display_ = new DoubleLcdDisplay(property_, true);
	layout->addWidget(lcd_display_, 0, Qt::AlignRight);

	QHBoxLayout *controls_layout = new QHBoxLayout();

	spin_box_ = new DoubleSpinBox(property_, true, true);
	controls_layout->addWidget(spin_box_);

	knob_ = new DoubleKnob(property_, true, true);
	controls_layout->addWidget(knob_);

	layout->addItem(controls_layout);

	this->setLayout(layout);
}

QVariant DoubleControl::variant_value() const
{
	return QVariant(spin_box_->value());
}

/*
void DoubleControl::change_value(const double value)
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

void DoubleControl::on_value_changed(const double value)
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
*/

} // namespace widgets
} // namespace ui
} // namespace sv
