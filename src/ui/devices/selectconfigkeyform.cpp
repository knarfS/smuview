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

#include <memory>

#include <QDebug>
#include <QVariant>

#include "selectconfigkeyform.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/devices/configkeycombobox.hpp"
#include "src/ui/devices/configurablecombobox.hpp"
#include "src/ui/devices/devicecombobox.hpp"

using std::shared_ptr;
using sv::devices::ConfigKey;

Q_DECLARE_METATYPE(sv::devices::ConfigKey)

namespace sv {
namespace ui {
namespace devices {

SelectConfigKeyForm::SelectConfigKeyForm(
		const Session &session,
		const bool show_getable_config_keys,
		const bool show_setable_config_keys,
		const bool show_listable_config_keys, QWidget *parent) :
	QFormLayout(parent),
	session_(session),
	show_getable_config_keys_(show_getable_config_keys),
	show_setable_config_keys_(show_setable_config_keys),
	show_listable_config_keys_(show_listable_config_keys)
{
	setup_ui();
	connect_signals();
}

void SelectConfigKeyForm::select_device(
	shared_ptr<sv::devices::BaseDevice> device)
{
	device_box_->select_device(device);
}

shared_ptr<sv::devices::Configurable>
	SelectConfigKeyForm::selected_configurable() const
{
	return configurable_box_->selected_configurable();
}

sv::devices::ConfigKey SelectConfigKeyForm::selected_config_key() const
{
	return config_key_box_->selected_config_key();
}

shared_ptr<sv::devices::properties::BaseProperty>
	SelectConfigKeyForm::get_property() const
{
	if (configurable_box_->selected_configurable() == nullptr)
		return nullptr;
	if (config_key_box_->selected_config_key() == ConfigKey::Unknown)
		return nullptr;

	return configurable_box_->selected_configurable()->get_property(
		config_key_box_->selected_config_key());
}

void SelectConfigKeyForm::setup_ui()
{
	device_box_ = new DeviceComboBox(session_);
	this->addRow(tr("Device"), device_box_);

	configurable_box_ = new ConfigurableComboBox(session_,
		device_box_->selected_device());
	this->addRow(tr("Configurable"), configurable_box_);

	config_key_box_ = new ConfigKeyComboBox(
		configurable_box_->selected_configurable(), show_getable_config_keys_,
		show_setable_config_keys_, show_listable_config_keys_);
	this->addRow(tr("Config key"), config_key_box_);
}

void SelectConfigKeyForm::connect_signals()
{
	connect(device_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_device_changed()));
	connect(configurable_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_configurable_changed()));
	connect(config_key_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_config_key_changed()));
}

void SelectConfigKeyForm::on_device_changed()
{
	configurable_box_->change_device(device_box_->selected_device());
}

void SelectConfigKeyForm::on_configurable_changed()
{
	config_key_box_->change_configurable(
		configurable_box_->selected_configurable());
}

void SelectConfigKeyForm::on_config_key_changed()
{
	Q_EMIT current_config_key_changed();
}

} // namespace devices
} // namespace ui
} // namespace sv
