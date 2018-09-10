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

#include "uint64spinbox.hpp"
#include "src/util.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/properties/uint64property.hpp"

namespace sv {
namespace ui {
namespace datatypes {

UInt64SpinBox::UInt64SpinBox(
		shared_ptr<devices::properties::UInt64Property> uint64_prop,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QSpinBox(parent),
	auto_commit_(auto_commit),
	auto_update_(auto_update),
	uint64_prop_(uint64_prop)
{
	setup_ui();
	connect_signals();
}

void UInt64SpinBox::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (uint64_prop_->is_listable()) {
		this->setRange(uint64_prop_->min(), uint64_prop_->max());
		this->setSingleStep(uint64_prop_->step());
	}
	this->setDisabled(!uint64_prop_->is_setable());
}

void UInt64SpinBox::connect_signals()
{
	// Widget -> Property
	if (auto_commit_ && uint64_prop_->is_setable()) {
		connect(this, SIGNAL(valueChanged(double)),
			this, SLOT(value_changed(const double)));
	}

	// Property -> Widget
	if (auto_update_) {
		connect(uint64_prop_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

void UInt64SpinBox::value_changed(const uint64_t value)
{
	uint64_prop_->value_changed(QVariant((qulonglong)value));
}

void UInt64SpinBox::on_value_changed(const QVariant value)
{
	this->setValue(value.toULongLong());
}

} // namespace datatypes
} // namespace ui
} // namespace sv
