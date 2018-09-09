/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#include "floatspinbox.hpp"
#include "src/util.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/properties/floatproperty.hpp"

namespace sv {
namespace ui {
namespace datatypes {

FloatSpinBox::FloatSpinBox(
		shared_ptr<devices::properties::FloatProperty> float_prop,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QDoubleSpinBox(parent),
	auto_commit_(auto_commit),
	auto_update_(auto_update),
	float_prop_(float_prop)
{
	setup_ui();
	connect_signals();
}

void FloatSpinBox::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (float_prop_->is_listable()) {
		this->setRange(float_prop_->min(), float_prop_->max());
		this->setSingleStep(float_prop_->step());
		this->setDecimals(float_prop_->decimal_places());
	}
	else {
		this->setDecimals(3);
	}
	/*
	if (unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless) {
		this->setSuffix(QString(" %1").arg(data::datautil::format_unit(unit_)));
	}
	*/
	this->setDisabled(!float_prop_->is_setable());
}

void FloatSpinBox::connect_signals()
{
	// Widget -> Property
	if (auto_commit_ && float_prop_->is_setable()) {
		connect(this, SIGNAL(valueChanged(double)),
			this, SLOT(value_changed(const double)));
	}

	// Property -> Widget
	if (auto_update_) {
		connect(float_prop_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

void FloatSpinBox::value_changed(const double value)
{
	float_prop_->value_changed(QVariant(value));
}

void FloatSpinBox::on_value_changed(const QVariant value)
{
	this->setValue(value.toDouble());
}

} // namespace datatypes
} // namespace ui
} // namespace sv
