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

#include "configkeycombobox.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/configurable.hpp"

Q_DECLARE_METATYPE(sv::devices::ConfigKey)

namespace sv {
namespace widgets {

ConfigKeyComboBox::ConfigKeyComboBox(
		shared_ptr<devices::Configurable> configurable,
		QWidget *parent) :
	QComboBox(parent),
	configurable_(configurable)
{
	setup_ui();
}

void ConfigKeyComboBox::set_configurable(
	shared_ptr<devices::Configurable> configurable)
{
	configurable_ = configurable;
	fill_config_keys();
}

devices::ConfigKey ConfigKeyComboBox::selected_config_key()
{
	QVariant data = this->currentData();
	return data.value<devices::ConfigKey>();
}

void ConfigKeyComboBox::setup_ui()
{
	fill_config_keys();
}

void ConfigKeyComboBox::fill_config_keys()
{
	this->clear();

	if (!configurable_)
		return;

	for (auto config_key : configurable_->available_setable_config_keys()) {
		this->addItem(
			devices::deviceutil::format_config_key(config_key),
			QVariant::fromValue(config_key));
	}
}

} // namespace widgets
} // namespace sv

