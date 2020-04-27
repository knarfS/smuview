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
#include <QLabel>

#include "stringlabel.hpp"
#include "src/util.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/stringproperty.hpp"
#include "src/devices/configurable.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace ui {
namespace datatypes {

StringLabel::StringLabel(
		shared_ptr<sv::data::properties::BaseProperty> property,
		const bool auto_update, QWidget *parent) :
	QLabel(parent),
	BaseWidget(property, false, auto_update)
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != data::DataType::String) {

		QString msg = QString("StringLabel with property of type ").append(
			data::datautil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	setup_ui();
	connect_signals();
}

void StringLabel::setup_ui()
{
	//this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (property_ != nullptr && property_->is_getable()) {
		shared_ptr<data::properties::StringProperty> string_prop =
			dynamic_pointer_cast<data::properties::StringProperty>(property_);

		this->setText(string_prop->string_value());
	}
	else {
		this->setDisabled(true);
		this->setText(tr("-"));
	}
}

void StringLabel::connect_signals()
{
	// Property -> Widget
	if (auto_update_ && property_ != nullptr) {
		connect(property_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

QVariant StringLabel::variant_value() const
{
	return QVariant(this->text());
}

void StringLabel::value_changed(const QString value)
{
	(void)value;
	// Nothing to do here.
}

void StringLabel::on_value_changed(const QVariant qvar)
{
	this->setText(qvar.toString());
}

void StringLabel::on_list_changed()
{
	// Nothing to do here.
}

} // namespace datatypes
} // namespace ui
} // namespace sv
