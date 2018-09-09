/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QApplication>

#include "boolbutton.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/properties/boolproperty.hpp"

namespace sv {
namespace ui {
namespace datatypes {

BoolButton::BoolButton(
		shared_ptr<devices::properties::BoolProperty> bool_prop,
		const bool auto_commit, const bool auto_update,
		QWidget *parent) :
	QPushButton(parent),
	auto_commit_(auto_commit),
	auto_update_(auto_update),
	on_icon_(":/icons/status-green.svg"),
	off_icon_(":/icons/status-red.svg"),
	dis_icon_(":/icons/status-grey.svg"),
	bool_prop_(bool_prop)
{
	setup_ui();
	connect_signals();
}

void BoolButton::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->setIconSize(QSize(8, 8));
	this->setDisabled(!bool_prop_->is_setable());
	if (bool_prop_->is_getable()) {
		on_value_changed(bool_prop_->value());
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
	if (auto_commit_ && bool_prop_->is_setable()) {
		connect(this, SIGNAL(clicked(bool)),
			this, SLOT(value_changed(const bool)));
	}

	// Property -> Widget
	if (auto_update_) {
		connect(bool_prop_.get(), SIGNAL(value_changed(const QVariant)),
			this, SLOT(on_value_changed(const QVariant)));
	}
}

void BoolButton::value_changed(const bool value)
{
	bool_prop_->value_changed(QVariant(value));
}

void BoolButton::on_value_changed(const QVariant value)
{
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
}

} // namespace datatypes
} // namespace ui
} // namespace sv
