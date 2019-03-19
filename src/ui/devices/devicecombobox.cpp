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

#include "devicecombobox.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"

using std::shared_ptr;

Q_DECLARE_METATYPE(shared_ptr<sv::devices::BaseDevice>)

namespace sv {
namespace ui {
namespace devices {

DeviceComboBox::DeviceComboBox(const Session &session, QWidget *parent) :
	QComboBox(parent),
	session_(session)
{
	setup_ui();
}

void DeviceComboBox::select_device(shared_ptr<sv::devices::BaseDevice> device)
{
	for (int i = 0; i < this->count(); ++i) {
		QVariant data = this->itemData(i, Qt::UserRole);
		auto item_device = data.value<shared_ptr<sv::devices::BaseDevice>>();
		if (item_device == device) {
			this->setCurrentIndex(i);
			break;
		}
	}
}

shared_ptr<sv::devices::BaseDevice> DeviceComboBox::selected_device() const
{
	QVariant data = this->currentData();
	return data.value<shared_ptr<sv::devices::BaseDevice>>();
}

void DeviceComboBox::setup_ui()
{
	for (const auto &device_pair : session_.devices()) {
		this->addItem(
			device_pair.second->full_name(),
			QVariant::fromValue(device_pair.second));
	}
}

} // namespace devices
} // namespace ui
} // namespace sv
