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

#include "viewhelper.hpp"
#include "src/session.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/demodmmcontrolview.hpp"
#include "src/ui/views/measurementcontrolview.hpp"
#include "src/ui/views/sourcesinkcontrolview.hpp"

using std::shared_ptr;
using sv::devices::ConfigKey;
using sv::devices::DeviceType;

namespace sv {
namespace ui {
namespace views {
namespace viewhelper {

BaseView *get_view_for_configurable(const Session &session,
	shared_ptr<sv::devices::Configurable> configurable)
{
	if (!configurable)
		return nullptr;

	// Power supplies or eleectronic loads control view
	if ((configurable->device_type() == DeviceType::PowerSupply ||
		configurable->device_type() == DeviceType::ElectronicLoad) &&
		(configurable->has_get_config(ConfigKey::Enabled) ||
		configurable->has_set_config(ConfigKey::Enabled) ||
		configurable->has_get_config(ConfigKey::Regulation) ||
		configurable->has_set_config(ConfigKey::Regulation) ||
		configurable->has_get_config(ConfigKey::VoltageTarget) ||
		configurable->has_set_config(ConfigKey::VoltageTarget) ||
		configurable->has_get_config(ConfigKey::CurrentLimit) ||
		configurable->has_set_config(ConfigKey::CurrentLimit) ||
		configurable->has_get_config(ConfigKey::OverVoltageProtectionEnabled) ||
		configurable->has_set_config(ConfigKey::OverVoltageProtectionEnabled) ||
		configurable->has_get_config(ConfigKey::OverVoltageProtectionThreshold) ||
		configurable->has_set_config(ConfigKey::OverVoltageProtectionThreshold) ||
		configurable->has_get_config(ConfigKey::OverCurrentProtectionEnabled) ||
		configurable->has_set_config(ConfigKey::OverCurrentProtectionEnabled) ||
		configurable->has_get_config(ConfigKey::OverCurrentProtectionThreshold) ||
		configurable->has_set_config(ConfigKey::OverCurrentProtectionThreshold) ||
		configurable->has_get_config(ConfigKey::UnderVoltageConditionEnabled) ||
		configurable->has_set_config(ConfigKey::UnderVoltageConditionEnabled) ||
		configurable->has_get_config(ConfigKey::UnderVoltageConditionThreshold) ||
		configurable->has_set_config(ConfigKey::UnderVoltageConditionThreshold))) {

		return new SourceSinkControlView(session, configurable);
	}

	// View for DemoDMM Device
	if (configurable->device_type() == DeviceType::DemoDev &&
		(configurable->has_get_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_set_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_get_config(ConfigKey::Amplitude) ||
		configurable->has_set_config(ConfigKey::Amplitude) /* ||
		configurable->has_get_config(ConfigKey::Offset) ||
		configurable->has_set_config(ConfigKey::Offset)	*/ )) {

		return new DemoDMMControlView(session, configurable);
	}

	// Measurement devices like DMMs, scales, LCR meters, etc.
	if ((configurable->device_type() == DeviceType::Multimeter ||
		configurable->device_type() == DeviceType::Multimeter ||
		configurable->device_type() == DeviceType::SoundLevelMeter ||
		configurable->device_type() == DeviceType::Thermometer ||
		configurable->device_type() == DeviceType::Hygrometer ||
		configurable->device_type() == DeviceType::Energymeter ||
		configurable->device_type() == DeviceType::LcrMeter ||
		configurable->device_type() == DeviceType::Scale ||
		configurable->device_type() == DeviceType::Powermeter) &&
		(configurable->has_get_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_set_config(ConfigKey::MeasuredQuantity))) {

		return new MeasurementControlView(session, configurable);
	}

	return nullptr;
}

} // namespace viewhelper
} // namespace views
} // namespace ui
} // namespace sv
