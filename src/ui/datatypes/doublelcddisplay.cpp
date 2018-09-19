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

#include <stdexcept>

#include <QDebug>

#include "doublelcddisplay.hpp"
#include "src/util.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/properties/baseproperty.hpp"
#include "src/devices/properties/doubleproperty.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace ui {
namespace datatypes {

DoubleLcdDisplay::DoubleLcdDisplay(
		shared_ptr<sv::devices::properties::BaseProperty> property,
		 const bool auto_update, QWidget *parent) :
	widgets::LcdDisplay(5/*Dummy*/, 3/*Dummy*/, false,
		QString("Dummy"), QString(""), QString(""), false, parent),
	BaseWidget(property, false, auto_update)
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != devices::DataType::Double) {

		QString msg = QString("DoubleLcdDisplay with property of type ").append(
			devices::deviceutil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	setup_ui();
	connect_signals();
}

void DoubleLcdDisplay::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (property_ != nullptr && property_->is_listable()) {
		shared_ptr<devices::properties::DoubleProperty> double_prop =
			dynamic_pointer_cast<devices::properties::DoubleProperty>(property_);

		this->set_digits(double_prop->digits(), double_prop->decimal_places());
	}
	if (property_ != nullptr && property_->unit() != data::Unit::Unknown &&
			property_->unit() != data::Unit::Unitless) {
		this->set_unit(
			QString(" %1").arg(data::datautil::format_unit(property_->unit())));
	}
	if (property_ != nullptr && property_->is_getable())
		on_value_changed(property_->value());
	else
		on_value_changed(QVariant(.0));
}

void DoubleLcdDisplay::connect_signals()
{
	// Property -> Widget
	if (auto_update_ && property_ != nullptr) {
		connect(property_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

QVariant DoubleLcdDisplay::variant_value() const
{
	return QVariant(this->value());
}

void DoubleLcdDisplay::value_changed(const double value)
{
	(void)value;
}

void DoubleLcdDisplay::on_value_changed(const QVariant qvar)
{
	this->set_value(qvar.toDouble());
}

} // namespace datatypes
} // namespace ui
} // namespace sv
