/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <QApplication>
#include <QDebug>

#include "boolbutton.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace ui {
namespace datatypes {

BoolButton::BoolButton(
		shared_ptr<sv::data::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QPushButton(parent),
	BaseWidget(property, auto_commit, auto_update),
	on_icon_(":/icons/status-green.svg"),
	off_icon_(":/icons/status-red.svg"),
	dis_icon_(":/icons/status-grey.svg")
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != data::DataType::Bool) {

		QString msg = QString("BoolButton with property of type ").append(
			data::datautil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	setup_ui();
	connect_signals();
}

void BoolButton::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->setIconSize(QSize(8, 8));
	this->setCheckable(true);
	if (property_ == nullptr || !property_->is_setable())
		this->setDisabled(true);
	if (property_ != nullptr && property_->is_getable()) {
		on_value_changed(property_->value());
	}
	else {
		this->setIcon(dis_icon_);
		this->setText(tr("On/Off"));
		this->setChecked(false);
	}
}

void BoolButton::connect_signals()
{
	// Widget -> Property
	connect_widget_2_prop_signals();

	// Property -> Widget (no ckeck for getable, comes via meta package!)
	if (auto_update_ && property_ != nullptr) {
		connect(property_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

void BoolButton::connect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		connect(this, SIGNAL(toggled(bool)),
			this, SLOT(value_changed(const bool)));
	}
}

void BoolButton::disconnect_widget_2_prop_signals()
{
	if (auto_commit_ && property_ != nullptr && property_->is_setable()) {
		disconnect(this, SIGNAL(toggled(bool)),
			this, SLOT(value_changed(const bool)));
	}
}

QVariant BoolButton::variant_value() const
{
	return QVariant(this->isChecked());
}

void BoolButton::value_changed(const bool value)
{
	if (property_ != nullptr)
		property_->change_value(QVariant(value));
}

void BoolButton::on_value_changed(const QVariant value)
{
	// Disconnect Widget -> Property signal to prevent echoing
	disconnect_widget_2_prop_signals();

	if (value.toBool()) {
		this->setIcon(on_icon_);
		this->setText(tr("On"));
		this->setChecked(true);
	}
	else {
		this->setIcon(off_icon_);
		this->setText(tr("Off"));
		this->setChecked(false);
	}

	connect_widget_2_prop_signals();
}

void BoolButton::on_list_changed()
{
	// Nothing to do here.
}

} // namespace datatypes
} // namespace ui
} // namespace sv
