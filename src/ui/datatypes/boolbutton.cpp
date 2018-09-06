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

namespace sv {
namespace ui {
namespace datatypes {

BoolButton::BoolButton(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key, bool auto_commit,
		QWidget *parent) :
	QPushButton(parent),
	on_icon_(":/icons/status-green.svg"),
	off_icon_(":/icons/status-red.svg"),
	dis_icon_(":/icons/status-grey.svg"),
	configurable_(configurable),
	config_key_(config_key),
	auto_commit_(auto_commit)
{
	setup_ui();
	connect_signals();
}

void BoolButton::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->setIconSize(QSize(8, 8));
	this->setDisabled(!configurable_->has_set_config(config_key_));
	if (!configurable_->has_get_config(config_key_)) {
		this->setIcon(dis_icon_);
		this->setText(tr("On/Off"));
		this->setChecked(false);
	}
	else {
		change_state(configurable_->get_config<bool>(config_key_));
	}
}

void BoolButton::connect_signals()
{
	if (auto_commit_ && configurable_->has_set_config(config_key_))
		connect(this, SIGNAL(clicked(bool)),
			this, SLOT(on_state_changed(bool)));
}

void BoolButton::change_state(const bool value)
{
	if (value) {
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

void BoolButton::on_state_changed(const bool value)
{
	change_state(!value);
	Q_EMIT state_changed(value);
}

} // namespace datatypes
} // namespace ui
} // namespace sv
