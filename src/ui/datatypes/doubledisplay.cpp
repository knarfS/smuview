/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2022 Frank Stettner <frank-stettner@gmx.net>
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

#include "doubledisplay.hpp"
#include "src/util.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/doubleproperty.hpp"
#include "src/devices/configurable.hpp"
#include "src/ui/widgets/monofontdisplay.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace ui {
namespace datatypes {

DoubleDisplay::DoubleDisplay(
		shared_ptr<sv::data::properties::BaseProperty> property,
		const bool auto_update, QWidget *parent) :
	widgets::MonoFontDisplay(widgets::MonoFontDisplayType::FixedRange,
		QString(""), QString(""), QString(""), false, parent),
	BaseWidget(property, false, auto_update)
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != data::DataType::Double) {

		QString msg = QString("DoubleDisplay with property of type ").append(
			data::datautil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	DoubleDisplay::setup_ui();
	connect_signals();
}

void DoubleDisplay::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (property_ != nullptr && property_->is_listable()) {
		shared_ptr<data::properties::DoubleProperty> double_prop =
			dynamic_pointer_cast<data::properties::DoubleProperty>(property_);
		this->set_decimal_places(double_prop->total_digits(), double_prop->decimal_places());
	}
	if (property_ != nullptr && property_->unit() != data::Unit::Unknown &&
			property_->unit() != data::Unit::Unitless) {
		this->set_unit(data::datautil::format_unit(property_->unit()));
	}
	if (property_ != nullptr && property_->is_getable())
		on_value_changed(property_->value());
	else
		on_value_changed(QVariant(.0));
}

void DoubleDisplay::connect_signals()
{
	// Property -> Widget
	if (auto_update_ && property_ != nullptr) {
		connect(property_.get(), &data::properties::BaseProperty::value_changed,
			this, &DoubleDisplay::on_value_changed);
		connect(property_.get(), &data::properties::BaseProperty::list_changed,
			this, &DoubleDisplay::on_list_changed);
	}
}

QVariant DoubleDisplay::variant_value() const
{
	return QVariant(this->value());
}

void DoubleDisplay::value_changed(const double value)
{
	(void)value;
	// Nothing to do here.
}

void DoubleDisplay::on_value_changed(const QVariant &qvar)
{
	this->set_value(qvar.toDouble());
}

void DoubleDisplay::on_list_changed()
{
	if (property_ != nullptr && property_->is_listable()) {
		shared_ptr<data::properties::DoubleProperty> double_prop =
			dynamic_pointer_cast<data::properties::DoubleProperty>(property_);
		this->set_decimal_places(double_prop->total_digits(), double_prop->decimal_places());

		if (property_->is_getable())
			this->set_value(double_prop->double_value());
	}
}

} // namespace datatypes
} // namespace ui
} // namespace sv
