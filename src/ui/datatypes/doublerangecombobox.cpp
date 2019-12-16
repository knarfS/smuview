/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#include "doublerangecombobox.hpp"
#include "src/util.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/doublerangeproperty.hpp"
#include "src/devices/configurable.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace ui {
namespace datatypes {

DoubleRangeComboBox::DoubleRangeComboBox(
		shared_ptr<sv::data::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QComboBox(parent),
	BaseWidget(property, auto_commit, auto_update)
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != data::DataType::DoubleRange) {

		QString msg = QString("DoubleRangeComboBox with property of type ").append(
			data::datautil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	setup_ui();
	connect_signals();
}

void DoubleRangeComboBox::setup_ui()
{
	//this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (property_ != nullptr && property_->is_listable()) {
		shared_ptr<data::properties::DoubleRangeProperty> doublerange_prop =
			dynamic_pointer_cast<data::properties::DoubleRangeProperty>(property_);

		for (const auto &doublerange : doublerange_prop->list_values()) {
			this->addItem(
				doublerange_prop->to_string(doublerange),
				QVariant::fromValue(doublerange));
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

void DoubleRangeComboBox::connect_signals()
{
	// Widget -> Property
	connect_widget_2_prop_signals();

	// Property -> Widget
	if (auto_update_ && property_ != nullptr) {
		connect(property_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
		connect(property_.get(), &data::properties::BaseProperty::list_changed,
			this, &DoubleRangeComboBox::on_list_changed);
	}
}

void DoubleRangeComboBox::connect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		connect(this, SIGNAL(currentIndexChanged(int)),
			this, SLOT(value_changed(int)));
	}
}

void DoubleRangeComboBox::disconnect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		disconnect(this, SIGNAL(currentIndexChanged(int)),
			this, SLOT(value_changed(int)));
	}
}

QVariant DoubleRangeComboBox::variant_value() const
{
	return this->currentData();
}

void DoubleRangeComboBox::value_changed(int index)
{
	(void)index;

	if (property_ != nullptr) {
		property_->change_value(this->currentData());
	}
}

void DoubleRangeComboBox::on_value_changed(const QVariant value)
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	shared_ptr<data::properties::DoubleRangeProperty> doublerange_prop =
		dynamic_pointer_cast<data::properties::DoubleRangeProperty>(property_);
	this->setCurrentText(doublerange_prop->to_string(value));;

	connect_widget_2_prop_signals();
}

void DoubleRangeComboBox::on_list_changed()
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	if (property_ != nullptr && property_->is_listable()) {
		this->clear();

		shared_ptr<data::properties::DoubleRangeProperty> doublerange_prop =
			dynamic_pointer_cast<data::properties::DoubleRangeProperty>(property_);
		for (const auto &doublerange : doublerange_prop->list_values()) {
			this->addItem(
				doublerange_prop->to_string(doublerange),
				QVariant::fromValue(doublerange));
		}

		if (property_->is_getable())
			this->setCurrentText(doublerange_prop->to_string(property_->value()));
	}

	connect_widget_2_prop_signals();
}

} // namespace datatypes
} // namespace ui
} // namespace sv
