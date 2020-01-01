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

#include <memory>
#include <set>

#include <QDebug>
#include <QVariant>

#include "selectpropertyform.hpp"
#include "src/session.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/devices/configkeycombobox.hpp"
#include "src/ui/devices/configurablecombobox.hpp"
#include "src/ui/devices/devicecombobox.hpp"

using std::set;
using std::shared_ptr;

namespace sv {
namespace ui {
namespace devices {

SelectPropertyForm::SelectPropertyForm(
		const Session &session, QWidget *parent) :
	QFormLayout(parent),
	session_(session)
{
	setup_ui();
	connect_signals();
}

void SelectPropertyForm::filter_config_keys(set<sv::data::DataType> data_types)
{
	config_key_box_->filter_config_keys(data_types);
}

void SelectPropertyForm::select_device(
	shared_ptr<sv::devices::BaseDevice> device)
{
	device_box_->select_device(device);
}

void SelectPropertyForm::select_configurable(
	shared_ptr<sv::devices::Configurable> configurable)
{
	configurable_box_->select_configurable(configurable);
}

void SelectPropertyForm::select_config_key(
	sv::devices::ConfigKey config_key)
{
	config_key_box_->select_config_key(config_key);
}

shared_ptr<sv::devices::BaseDevice> SelectPropertyForm::selected_device() const
{
	return device_box_->selected_device();
}

shared_ptr<sv::devices::Configurable>
	SelectPropertyForm::selected_configurable() const
{
	return configurable_box_->selected_configurable();
}

shared_ptr<sv::data::properties::BaseProperty>
	SelectPropertyForm::selected_property() const
{
	return configurable_box_->selected_configurable()->get_property(
		config_key_box_->selected_config_key());
}

sv::devices::ConfigKey SelectPropertyForm::selected_config_key() const
{
	return config_key_box_->selected_config_key();
}

void SelectPropertyForm::setup_ui()
{
	device_box_ = new DeviceComboBox(session_);
	this->addRow(tr("Device"), device_box_);

	configurable_box_ = new ConfigurableComboBox(
		device_box_->selected_device());
	this->addRow(tr("Configurable"), configurable_box_);

	config_key_box_ = new ConfigKeyComboBox(
		configurable_box_->selected_configurable());
	this->addRow(tr("Config Key"), config_key_box_);
}

void SelectPropertyForm::connect_signals()
{
	connect(device_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_device_changed()));
	connect(configurable_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_configurable_changed()));
}

void SelectPropertyForm::on_device_changed()
{
	configurable_box_->change_device(device_box_->selected_device());
}

void SelectPropertyForm::on_configurable_changed()
{
	config_key_box_->change_configurable(
		configurable_box_->selected_configurable());
}

} // namespace devices
} // namespace ui
} // namespace sv
