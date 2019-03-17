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

#include "configurablecombobox.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"

using std::dynamic_pointer_cast;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace devices {

ConfigurableComboBox::ConfigurableComboBox(const Session &session,
		shared_ptr<sv::devices::BaseDevice> device,
		QWidget *parent) :
	QComboBox(parent),
	session_(session),
	device_(device)
{
	setup_ui();
}

void ConfigurableComboBox::select_configurable(
	shared_ptr<sv::devices::Configurable> configuable)
{
	for (int i = 0; i < this->count(); ++i) {
		QVariant data = this->itemData(i, Qt::UserRole);
		auto item_config = data.value<shared_ptr<sv::devices::Configurable>>();
		if (item_config == configuable) {
			this->setCurrentIndex(i);
			break;
		}
	}
}

shared_ptr<sv::devices::Configurable>
	ConfigurableComboBox::selected_configurable() const
{
	QVariant data = this->currentData();
	return data.value<shared_ptr<sv::devices::Configurable>>();
}

void ConfigurableComboBox::setup_ui()
{
	if (device_ == nullptr)
		return;

	auto hw_device = dynamic_pointer_cast<sv::devices::HardwareDevice>(device_);
	if (!hw_device)
		return;

	for (const auto &c_pair : hw_device->configurable_map()) {
		// Only show configurables that either are getable, setable or listable.
		auto configurable = c_pair.second;
		if (!configurable->is_controllable())
			continue;

		this->addItem(
			configurable->display_name(),
			QVariant::fromValue(configurable));
	}

}

void ConfigurableComboBox::change_device(
	shared_ptr<sv::devices::BaseDevice> device)
{
	device_ = device;
	for (int i = this->count(); i >= 0; --i)
		this->removeItem(i);
	this->setup_ui();
}

} // namespace devices
} // namespace ui
} // namespace sv
