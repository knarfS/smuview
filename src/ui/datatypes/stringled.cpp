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
#include <QHBoxLayout>

#include "stringled.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace ui {
namespace datatypes {

StringLed::StringLed(
		shared_ptr<sv::data::properties::BaseProperty> property,
		const bool auto_update,
		const QIcon &on_icon, const QIcon &off_icon, const QIcon &dis_icon,
		const QString &on_value, const QString &off_value,
		const QString &text, QWidget *parent) :
	QWidget(parent),
	BaseWidget(property, false, auto_update),
	on_icon_(on_icon),
	off_icon_(off_icon),
	dis_icon_(dis_icon),
	on_value_(on_value),
	off_value_(off_value),
	text_(text)
{
	// Check property
	if (property_ != nullptr &&
			property_->data_type() != data::DataType::String) {

		QString msg = QString("StringLed with property of type ").append(
			data::datautil::format_data_type(property_->data_type()));
		throw std::runtime_error(msg.toStdString());
	}

	setup_ui();
	connect_signals();
}

void StringLed::setup_ui()
{
	QHBoxLayout *layout = new QHBoxLayout();

	// Led icon
	led_label_ = new QLabel();
	if (property_ != nullptr && property_->is_getable()) {
		on_value_changed(property_->value());
	}
	else {
		led_label_->setPixmap(
			dis_icon_.pixmap(16, 16, QIcon::Mode::Disabled, QIcon::State::Off));
	}
	layout->addWidget(led_label_);

	// Text
	if (property_ != nullptr && text_ == nullptr) {
		text_ = devices::deviceutil::format_config_key(
			property_->config_key());
	}
	else if (property_ == nullptr && text_ == nullptr) {
		text_ = devices::deviceutil::format_config_key(
			devices::ConfigKey::Unknown);
	}
	text_label_ = new QLabel(text_);
	if (property_ == nullptr || !property_->is_getable())
		text_label_->setDisabled(true);
	layout->addWidget(text_label_);

	this->setLayout(layout);
}

void StringLed::connect_signals()
{
	// Property -> Widget (no ckeck for getable, comes via meta package!)
	if (property_ != nullptr && auto_update_) {
		connect(property_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

QVariant StringLed::variant_value() const
{
	// TODO: Member variable with actual state!
	return QVariant(QString(""));
}

void StringLed::value_changed(const bool value)
{
	(void)value;
	// Nothing to do here.
}

void StringLed::on_value_changed(const QVariant &qvar)
{
	QString value = qvar.toString();
	if (on_value_ != nullptr && value == on_value_) {
		led_label_->setPixmap(
			on_icon_.pixmap(16, 16, QIcon::Mode::Active, QIcon::State::On));
	}
	else if (off_value_ != nullptr && value == off_value_) {
		led_label_->setPixmap(
			off_icon_.pixmap(16, 16, QIcon::Mode::Active, QIcon::State::Off));
	}
	else {
		led_label_->setPixmap(
			dis_icon_.pixmap(16, 16, QIcon::Mode::Active, QIcon::State::Off));
	}
}

void StringLed::on_list_changed()
{
	// Nothing to do here.
}

} // namespace datatypes
} // namespace ui
} // namespace sv
