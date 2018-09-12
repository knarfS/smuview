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

#include "boolcheckbox.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/properties/baseproperty.hpp"

namespace sv {
namespace ui {
namespace datatypes {

BoolCheckBox::BoolCheckBox(
		shared_ptr<devices::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QCheckBox(parent),
	BaseWidget(property, auto_commit, auto_update)
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != devices::DataType::Bool) {

		QString msg = QString("BoolCheckBox with property of type ").append(
			devices::deviceutil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	setup_ui();
	connect_signals();
}

void BoolCheckBox::setup_ui()
{
	if (property_ == nullptr || !property_->is_setable())
		this->setDisabled(true);
	if (property_ != nullptr && property_->is_getable())
		on_value_changed(property_->value());
	else
		on_value_changed(QVariant(false));
}

void BoolCheckBox::connect_signals()
{
	// Widget -> Property
	connect_widget_2_prop_signals();

	// Property -> Widget (no ckeck for getable, comes via meta package!)
	if (auto_update_ && property_ != nullptr) {
		connect(property_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

void BoolCheckBox::connect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		connect(this, SIGNAL(stateChanged(bool)),
			this, SLOT(value_changed(const bool)));
	}
}

void BoolCheckBox::disconnect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		disconnect(this, SIGNAL(stateChanged(bool)),
			this, SLOT(value_changed(const bool)));
	}
}

QVariant BoolCheckBox::variant_value() const
{
	return QVariant(this->isChecked());
}

void BoolCheckBox::value_changed(const bool value)
{
	if (property_ != nullptr)
		property_->change_value(QVariant(value));
}

void BoolCheckBox::on_value_changed(const QVariant value)
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	this->setChecked(value.toBool());

	connect_widget_2_prop_signals();
}

} // namespace datatypes
} // namespace ui
} // namespace sv
