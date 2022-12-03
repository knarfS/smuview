/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include "int32spinbox.hpp"
#include "src/util.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/int32property.hpp"
#include "src/devices/configurable.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace ui {
namespace datatypes {

Int32SpinBox::Int32SpinBox(
		shared_ptr<sv::data::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QSpinBox(parent),
	BaseWidget(property, auto_commit, auto_update)
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != data::DataType::Int32) {

		QString msg = QString("Int32SpinBox with property of type ").append(
			data::datautil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	setup_ui();
	connect_signals();
}

void Int32SpinBox::setup_ui()
{
	this->setAlignment(Qt::AlignRight);
	if (property_ != nullptr && property_->is_listable()) {
		shared_ptr<data::properties::Int32Property> int32_prop =
			dynamic_pointer_cast<data::properties::Int32Property>(property_);

		this->setRange(int32_prop->min(), int32_prop->max());
		this->setSingleStep(int32_prop->step());
	}
	if (property_ != nullptr && property_->unit() != data::Unit::Unknown &&
			property_->unit() != data::Unit::Unitless) {
		this->setSuffix(
			QString(" %1").arg(data::datautil::format_unit(property_->unit())));
	}
	if (property_ == nullptr || !property_->is_setable())
		this->setDisabled(true);
	if (property_ != nullptr && property_->is_getable())
		on_value_changed(property_->value());
	else
		on_value_changed(QVariant(0));
}

void Int32SpinBox::connect_signals()
{
	// Widget -> Property
	connect_widget_2_prop_signals();

	// Property -> Widget
	if (auto_update_ && property_ != nullptr) {
		connect(property_.get(), &data::properties::BaseProperty::value_changed,
			this, &Int32SpinBox::on_value_changed);
		connect(property_.get(), &data::properties::BaseProperty::list_changed,
			this, &Int32SpinBox::on_list_changed);
	}
}

void Int32SpinBox::connect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		connect(this, SIGNAL(editingFinished()),
			this, SLOT(value_changed()));
	}
}

void Int32SpinBox::disconnect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		disconnect(this, SIGNAL(editingFinished()),
			this, SLOT(value_changed()));
	}
}

QVariant Int32SpinBox::variant_value() const
{
	return QVariant(this->value());
}

void Int32SpinBox::value_changed()
{
	if (property_ != nullptr)
		property_->change_value(QVariant(this->value()));
}

void Int32SpinBox::on_value_changed(const QVariant &qvar)
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	this->setValue(qvar.toInt());

	connect_widget_2_prop_signals();
}

void Int32SpinBox::on_list_changed()
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	if (property_ != nullptr && property_->is_listable()) {
		shared_ptr<data::properties::Int32Property> int32_prop =
			dynamic_pointer_cast<data::properties::Int32Property>(property_);
		this->setRange(int32_prop->min(), int32_prop->max());
		this->setSingleStep(int32_prop->step());

		if (property_->is_getable())
			this->setValue(int32_prop->int32_value());
	}

	connect_widget_2_prop_signals();
}

} // namespace datatypes
} // namespace ui
} // namespace sv
