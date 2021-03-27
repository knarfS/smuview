/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include "oscilloscopedevice.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/configurable.hpp"

using std::static_pointer_cast;

namespace sv {
namespace devices {

OscilloscopeDevice::OscilloscopeDevice(
		const shared_ptr<sigrok::Context> sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device) :
	HardwareDevice(sr_context, sr_device)
{
}

void OscilloscopeDevice::init_configurables()
{
	HardwareDevice::init_configurables();

	for (const auto &c_pair : configurable_map_) {
		auto configurable = c_pair.second;

		// Check if the device has the config key "VDiv". If so, each possible
		// value of the config key "ProbeFactor" could have a different
		// listing for "VDiv"!
		if (configurable->property_map().count(ConfigKey::ProbeFactor) > 0 &&
			configurable->property_map().count(ConfigKey::VDiv) > 0) {

			auto pf_property = configurable->property_map()[ConfigKey::ProbeFactor];
			auto vdiv_property = configurable->property_map()[ConfigKey::VDiv];
			connect(
				pf_property.get(), &data::properties::BaseProperty::value_changed,
				vdiv_property.get(), &data::properties::BaseProperty::list_config);
		}
	}
}

void OscilloscopeDevice::init_channels()
{
	HardwareDevice::init_channels();
}

} // namespace devices
} // namespace sv
