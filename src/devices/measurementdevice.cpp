/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "measurementdevice.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"

namespace sv {
namespace devices {

MeasurementDevice::MeasurementDevice(
		const shared_ptr<sigrok::Context> sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device) :
	HardwareDevice(sr_context, sr_device)
{
}

void MeasurementDevice::init_configurables()
{
	HardwareDevice::init_configurables();

	for (const auto &c_pair : configurable_map_) {
		auto configurable = c_pair.second;

		// Check if the device has the config key "Range". If so, each possible
		// value of the config key "MeasuredQuantity" could have a different
		// listing for "Range"!
		if (configurable->properties().count(ConfigKey::Range) > 0 &&
			configurable->properties().count(ConfigKey::MeasuredQuantity) > 0) {

			auto range_property = configurable->properties()[ConfigKey::Range];
			auto mq_property =
				configurable->properties()[ConfigKey::MeasuredQuantity];
			connect(
				mq_property.get(), &data::properties::BaseProperty::value_changed,
				range_property.get(), &data::properties::BaseProperty::list_config);
		}
	}
}

} // namespace devices
} // namespace sv
