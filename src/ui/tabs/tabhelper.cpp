/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>

#include <QMainWindow>

#include "tabhelper.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/measurementdevice.hpp"
#include "src/devices/sourcesinkdevice.hpp"
#include "src/devices/virtualdevice.hpp"
#include "src/ui/tabs/basetab.hpp"
#include "src/ui/tabs/measurementtab.hpp"
#include "src/ui/tabs/sourcesinktab.hpp"
#include "src/ui/tabs/virtualtab.hpp"

using std::shared_ptr;
using std::static_pointer_cast;
using sv::devices::DeviceType;

namespace sv {
namespace ui {
namespace tabs {
namespace tabhelper {

BaseTab *get_tab_for_device(Session &session,
	shared_ptr<devices::BaseDevice> device, QMainWindow *parent)
{
	if (!device)
		return nullptr;

	// Power supplies or electronic loads contro
	if (device->type() == DeviceType::PowerSupply ||
		device->type() == DeviceType::ElectronicLoad) {

		return new SourceSinkTab(session,
			static_pointer_cast<devices::SourceSinkDevice>(device), parent);
	}

	// Measurement devices like DMMs, scales, LCR meters, etc., but also
	// the demo device(s)
	if (device->type() == DeviceType::Multimeter ||
		device->type() == DeviceType::SoundLevelMeter ||
		device->type() == DeviceType::Thermometer ||
		device->type() == DeviceType::Hygrometer ||
		device->type() == DeviceType::Energymeter ||
		device->type() == DeviceType::LcrMeter ||
		device->type() == DeviceType::Scale ||
		device->type() == DeviceType::Powermeter ||
		device->type() == DeviceType::DemoDev) {

		return new MeasurementTab(session,
			static_pointer_cast<devices::MeasurementDevice>(device), parent);
	}

	// Virtual device tab
	if (device->type() == DeviceType::VirtualDevice) {
		return new VirtualTab(session,
			static_pointer_cast<devices::VirtualDevice>(device), parent);
	}

	return nullptr;
}

} // namespace tabhelper
} // namespace tabs
} // namespace ui
} // namespace sv
