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
#include <QVariant>

#include "measuredquantitycombobox.hpp"
#include "src/util.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/properties/measuredquantityproperty.hpp"

Q_DECLARE_METATYPE(sv::data::Quantity)
Q_DECLARE_METATYPE(sv::devices::Configurable::measured_quantity_t)

namespace sv {
namespace ui {
namespace datatypes {

MeasuredQuantityComboBox::MeasuredQuantityComboBox(
		shared_ptr<devices::properties::MeasuredQuantityProperty> mq_prop,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QComboBox(parent),
	auto_commit_(auto_commit),
	auto_update_(auto_update),
	mq_prop_(mq_prop)
{
	setup_ui();
	connect_signals();
}

void MeasuredQuantityComboBox::setup_ui()
{
	//this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (mq_prop_->is_listable()) {
		devices::Configurable::measured_quantity_list_t mq_list =
			mq_prop_->list_values();
		for (auto pair : mq_list) {
			data::Quantity qunatity = pair.first;
			this->addItem(
				data::datautil::format_quantity(qunatity),
				QVariant::fromValue(qunatity));
		}
	}
	else { // TODO: is_getable?
		devices::Configurable::measured_quantity_t mq =
			mq_prop_->measured_quantity_value();
		data::Quantity qunatity = mq.first;
		this->addItem(
			data::datautil::format_quantity(qunatity),
			QVariant::fromValue(qunatity));
	}
	this->setCurrentText(data::datautil::format_quantity(
		mq_prop_->measured_quantity_value().first));
	this->setDisabled(!mq_prop_->is_setable());
}

void MeasuredQuantityComboBox::connect_signals()
{
	// Widget -> Property
	if (auto_commit_ && mq_prop_->is_setable()) {
		connect(this, SIGNAL(currentIndexChanged(const QString)),
			this, SLOT(value_changed(const QString)));
	}

	// Property -> Widget
	if (auto_update_) {
		connect(mq_prop_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

void MeasuredQuantityComboBox::value_changed(
	const devices::Configurable::measured_quantity_t value)
{
	mq_prop_->value_changed(QVariant().fromValue(value));
}

void MeasuredQuantityComboBox::on_value_changed(const QVariant value)
{
	this->setCurrentText(data::datautil::format_quantity(
		value.value<devices::Configurable::measured_quantity_t>().first));
}

} // namespace datatypes
} // namespace ui
} // namespece sv
