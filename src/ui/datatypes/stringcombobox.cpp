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

#include "stringcombobox.hpp"
#include "src/util.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/stringproperty.hpp"
#include "src/devices/configurable.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace ui {
namespace datatypes {

StringComboBox::StringComboBox(
		shared_ptr<sv::data::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QComboBox(parent),
	BaseWidget(property, auto_commit, auto_update)
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != data::DataType::String) {

		QString msg = QString("StringComboBox with property of type ").append(
			data::datautil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	setup_ui();
	connect_signals();
}

void StringComboBox::setup_ui()
{
	//this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (property_ != nullptr && property_->is_listable()) {
		shared_ptr<data::properties::StringProperty> string_prop =
			dynamic_pointer_cast<data::properties::StringProperty>(property_);

		this->addItems(string_prop->list_values());
	}
	else {
		this->setEditable(true);
	}
	if (property_ == nullptr || !property_->is_setable())
		this->setDisabled(true);
	if (property_ != nullptr && property_->is_getable())
		on_value_changed(property_->value());
}

void StringComboBox::connect_signals()
{
	// Widget -> Property
	connect_widget_2_prop_signals();

	// Property -> Widget
	if (auto_update_ && property_ != nullptr) {
		connect(property_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
		connect(property_.get(), &data::properties::BaseProperty::list_changed,
			this, &StringComboBox::on_list_changed);
	}
}

void StringComboBox::connect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		if (property_->is_listable()) {
			connect(this, SIGNAL(currentIndexChanged(const QString)),
				this, SLOT(value_changed(const QString)));
		}
		else {
			connect(this, SIGNAL(editTextChanged(const QString)),
				this, SLOT(value_changed(const QString)));
		}
	}
}

void StringComboBox::disconnect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		if (property_->is_listable()) {
			disconnect(this, SIGNAL(currentIndexChanged(const QString)),
				this, SLOT(value_changed(const QString)));
		}
		else {
			disconnect(this, SIGNAL(editTextChanged(const QString)),
				this, SLOT(value_changed(const QString)));
		}
	}
}

QVariant StringComboBox::variant_value() const
{
	return QVariant(this->currentText());
}

void StringComboBox::value_changed(const QString &value)
{
	if (property_ != nullptr)
		property_->change_value(QVariant(value));
}

void StringComboBox::on_value_changed(const QVariant &qvar)
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	this->setCurrentText(qvar.toString());

	connect_widget_2_prop_signals();
}

void StringComboBox::on_list_changed()
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	if (property_ != nullptr && property_->is_listable()) {
		this->clear();

		shared_ptr<data::properties::StringProperty> string_prop =
			dynamic_pointer_cast<data::properties::StringProperty>(property_);
		this->addItems(string_prop->list_values());

		if (property_->is_getable())
			this->setCurrentText(string_prop->string_value());
	}

	connect_widget_2_prop_signals();
}

} // namespace datatypes
} // namespace ui
} // namespace sv
