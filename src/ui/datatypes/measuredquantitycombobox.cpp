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
#include <QVariant>

#include "measuredquantitycombobox.hpp"
#include "src/util.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/properties/baseproperty.hpp"
#include "src/devices/properties/measuredquantityproperty.hpp"

using std::dynamic_pointer_cast;

Q_DECLARE_METATYPE(sv::data::Quantity)
Q_DECLARE_METATYPE(sv::devices::Configurable::measured_quantity_t)

namespace sv {
namespace ui {
namespace datatypes {

MeasuredQuantityComboBox::MeasuredQuantityComboBox(
		shared_ptr<devices::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QComboBox(parent),
	BaseWidget(property, auto_commit, auto_update)
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != devices::DataType::MQ) {

		QString msg =
			QString("MeasuredQuantityComboBox with property of type ").append(
				devices::deviceutil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	setup_ui();
	connect_signals();
}

void MeasuredQuantityComboBox::setup_ui()
{
	//this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (property_ != nullptr && property_->is_listable()) {
		shared_ptr<devices::properties::MeasuredQuantityProperty> mq_prop =
			dynamic_pointer_cast<devices::properties::MeasuredQuantityProperty>(
				property_);

		auto mq_list = mq_prop->list_values();
		for (auto pair : mq_list) {
			data::Quantity qunatity = pair.first;
			this->addItem(
				data::datautil::format_quantity(qunatity),
				QVariant::fromValue(qunatity));
		}
	}
	else if (property_ != nullptr && property_->is_getable()) {
		data::Quantity qunatity = property_->value().
			value<devices::Configurable::measured_quantity_t>().first;
		this->addItem(
			data::datautil::format_quantity(qunatity),
			QVariant::fromValue(qunatity));
	}
	if (property_ == nullptr || !property_->is_setable())
		this->setDisabled(true);
	if (property_ != nullptr && property_->is_getable())
		on_value_changed(property_->value());
}

void MeasuredQuantityComboBox::connect_signals()
{
	// Widget -> Property
	connect_widget_2_prop_signals();

	// Property -> Widget
	if (auto_update_ && property_ != nullptr) {
		connect(property_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

void MeasuredQuantityComboBox::connect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		connect(this, SIGNAL(currentIndexChanged(const QString)),
			this, SLOT(value_changed(const QString)));
	}
}

void MeasuredQuantityComboBox::disconnect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		disconnect(this, SIGNAL(currentIndexChanged(const QString)),
			this, SLOT(value_changed(const QString)));
	}
}

QVariant MeasuredQuantityComboBox::variant_value() const
{
	return QVariant(this->currentData());
}

void MeasuredQuantityComboBox::value_changed(
	const devices::Configurable::measured_quantity_t value)
{
	if (property_ != nullptr)
		property_->change_value(QVariant().fromValue(value));
}

void MeasuredQuantityComboBox::on_value_changed(const QVariant value)
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	this->setCurrentText(data::datautil::format_quantity(
		value.value<devices::Configurable::measured_quantity_t>().first));

	connect_widget_2_prop_signals();
}

} // namespace datatypes
} // namespace ui
} // namespece sv
