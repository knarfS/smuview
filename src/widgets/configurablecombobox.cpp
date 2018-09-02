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
namespace widgets {

ConfigurableComboBox::ConfigurableComboBox(shared_ptr<Session> session,
		QWidget *parent) :
	QComboBox(parent),
	session_(session)
{
	setup_ui();
}

shared_ptr<devices::Configurable> ConfigurableComboBox::selected_configurable()
{
	QVariant data = this->currentData();
	return data.value<shared_ptr<devices::Configurable>>();
}

void ConfigurableComboBox::setup_ui()
{
	for (auto device : session_->devices()) {
		auto hw_device = dynamic_pointer_cast<devices::HardwareDevice>(device);
		if (!hw_device)
			continue;

		for (auto configurable : hw_device->configurables()) {
			//QString name = QString("%1 / %2").
			//	arg(device->name()).arg(configurable->name());
			this->addItem(
				configurable->name(), QVariant::fromValue(configurable));
		}
	}
}

} // namespace widgets
} // namespace sv

