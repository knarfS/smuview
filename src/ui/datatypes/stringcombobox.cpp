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

#include "stringcombobox.hpp"
#include "src/util.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/properties/stringproperty.hpp"

namespace sv {
namespace ui {
namespace datatypes {

StringComboBox::StringComboBox(
		shared_ptr<devices::properties::StringProperty> string_prop,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QComboBox(parent),
	auto_commit_(auto_commit),
	auto_update_(auto_update),
	string_prop_(string_prop)
{
	setup_ui();
	connect_signals();
}

void StringComboBox::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (string_prop_->is_listable()) {
		this->addItems(string_prop_->list_values());
	}
	else {
		this->setEditable(true);
	}
	this->setCurrentText(string_prop_->string_value());
	this->setDisabled(!string_prop_->is_setable());
}

void StringComboBox::connect_signals()
{
	// Widget -> Property
	if (auto_commit_ && string_prop_->is_setable()) {
		if (string_prop_->is_listable()) {
			connect(this, SIGNAL(currentIndexChanged(const QString)),
				this, SLOT(value_changed(const QString)));
		}
		else {
			connect(this, SIGNAL(editTextChanged(const QString)),
				this, SLOT(value_changed(const QString)));
		}
	}

	// Property -> Widget
	if (auto_update_) {
		connect(string_prop_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

void StringComboBox::value_changed(const QString value)
{
	string_prop_->value_changed(QVariant(value));
}

void StringComboBox::on_value_changed(const QVariant value)
{
	this->setCurrentText(value.toString());
}

} // namespace datatypes
} // namespace ui
} // namespace sv
