/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include "rationalcombobox.hpp"
#include "src/util.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/rationalproperty.hpp"
#include "src/devices/configurable.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace ui {
namespace datatypes {

RationalComboBox::RationalComboBox(
		shared_ptr<sv::data::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QComboBox(parent),
	BaseWidget(property, auto_commit, auto_update)
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != data::DataType::RationalPeriod &&
			property_->data_type() != data::DataType::RationalVolt) {

		QString msg = QString("RationalComboBox with property of type ").append(
			data::datautil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	setup_ui();
	connect_signals();
}

void RationalComboBox::setup_ui()
{
	//this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (property_ != nullptr && property_->is_listable()) {
		shared_ptr<data::properties::RationalProperty> rational_prop =
			dynamic_pointer_cast<data::properties::RationalProperty>(
				property_);

		for (const auto &rational : rational_prop->list_values()) {
			this->addItem(
				rational_prop->to_string(rational),
				QVariant::fromValue(rational));
		}
	}
	else if (property_ != nullptr && property_->is_getable()) {
		this->addItem(property_->to_string(), property_->value());
	}
	if (property_ == nullptr || !property_->is_setable())
		this->setDisabled(true);
	if (property_ != nullptr && property_->is_getable())
		on_value_changed(property_->value());
}

void RationalComboBox::connect_signals()
{
	// Widget -> Property
	connect_widget_2_prop_signals();

	// Property -> Widget
	if (auto_update_ && property_ != nullptr) {
		connect(property_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
		connect(property_.get(), &data::properties::BaseProperty::list_changed,
			this, &RationalComboBox::on_list_changed);
	}
}

void RationalComboBox::connect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		connect(this, SIGNAL(currentIndexChanged(int)),
			this, SLOT(value_changed(int)));
	}
}

void RationalComboBox::disconnect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		disconnect(this, SIGNAL(currentIndexChanged(int)),
			this, SLOT(value_changed(int)));
	}
}

QVariant RationalComboBox::variant_value() const
{
	return QVariant(this->currentData());
}

void RationalComboBox::value_changed(int index)
{
	(void)index;

	if (property_ != nullptr) {
		data::rational_t value =
			this->currentData().value<data::rational_t>();
		property_->change_value(QVariant::fromValue(value));
	}
}

void RationalComboBox::on_value_changed(const QVariant qvar)
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	shared_ptr<data::properties::RationalProperty> rational_prop =
		dynamic_pointer_cast<data::properties::RationalProperty>(property_);
	this->setCurrentText(rational_prop->to_string(qvar));

	connect_widget_2_prop_signals();
}

void RationalComboBox::on_list_changed()
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	if (property_ != nullptr && property_->is_listable()) {
		this->clear();

		shared_ptr<data::properties::RationalProperty> rational_prop =
			dynamic_pointer_cast<data::properties::RationalProperty>(
				property_);

		for (const auto &rational : rational_prop->list_values()) {
			this->addItem(
				rational_prop->to_string(rational),
				QVariant::fromValue(rational));
		}

		if (property_->is_getable())
			this->setCurrentText(rational_prop->to_string(property_->value()));
	}

	connect_widget_2_prop_signals();
}

} // namespace datatypes
} // namespace ui
} // namespace sv
