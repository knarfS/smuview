/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include "selectconfigurableform.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/ui/devices/configurablecombobox.hpp"
#include "src/ui/devices/devicecombobox.hpp"

using std::shared_ptr;

Q_DECLARE_METATYPE(shared_ptr<sv::data::BaseSignal>)

namespace sv {
namespace ui {
namespace devices {

SelectConfigurableForm::SelectConfigurableForm(
		const Session &session, QWidget *parent) :
	QFormLayout(parent),
	session_(session)
{
	setup_ui();
	connect_signals();
}

void SelectConfigurableForm::select_device(
	shared_ptr<sv::devices::BaseDevice> device)
{
	device_box_->select_device(device);
}

shared_ptr<sv::devices::Configurable>
	SelectConfigurableForm::selected_configurable() const
{
	return configurable_box_->selected_configurable();
}

void SelectConfigurableForm::setup_ui()
{
	device_box_ = new DeviceComboBox(session_);
	this->addRow(tr("Device"), device_box_);

	configurable_box_ = new ConfigurableComboBox(
		device_box_->selected_device());
	this->addRow(tr("Configurable"), configurable_box_);
}

void SelectConfigurableForm::connect_signals()
{
	connect(device_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_device_changed()));
}

void SelectConfigurableForm::on_device_changed()
{
	configurable_box_->change_device(device_box_->selected_device());
}

} // namespace devices
} // namespace ui
} // namespace sv
