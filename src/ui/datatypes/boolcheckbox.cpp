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

#include "boolcheckbox.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace ui {
namespace datatypes {

BoolCheckBox::BoolCheckBox(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key, bool auto_commit, QWidget *parent) :
	QCheckBox(parent),
	configurable_(configurable),
	config_key_(config_key),
	auto_commit_(auto_commit)
{
}

void BoolCheckBox::setup_ui()
{
	this->setDisabled(!configurable_->has_set_config(config_key_));
}

void BoolCheckBox::connect_signals()
{
	if (auto_commit_ && configurable_->has_set_config(config_key_))
		connect(this, SIGNAL(stateChanged(bool)),
			this, SLOT(on_value_changed(bool)));
}

void BoolCheckBox::change_value(const bool value)
{
	this->setChecked(value);
}

void BoolCheckBox::on_value_changed(const bool value)
{
	configurable_->set_config<bool>(config_key_, value);
}

} // namespace datatypes
} // namespace ui
} // namespace sv
