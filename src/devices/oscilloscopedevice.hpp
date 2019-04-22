/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DEVICES_OSCILLOSCOPEDEVICE_HPP
#define DEVICES_OSCILLOSCOPEDEVICE_HPP

#include <memory>
#include <utility>

#include "src/devices/hardwaredevice.hpp"

using std::forward;
using std::shared_ptr;

namespace sigrok {
class Context;
class HardwareDevice;
}

namespace sv {
namespace devices {

class OscilloscopeDevice : public HardwareDevice
{
	Q_OBJECT

private:
	explicit OscilloscopeDevice(const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device);

public:
	template<typename ...Arg>
	shared_ptr<OscilloscopeDevice> static create(Arg&&...arg)
	{
		struct make_shared_enabler : public OscilloscopeDevice {
			make_shared_enabler(Arg&&...arg) : OscilloscopeDevice(forward<Arg>(arg)...) {}
		};

		return make_shared<make_shared_enabler>(forward<Arg>(arg)...);
	}

protected:
	void init_channels() override;

};

} // namespace devices
} // namespace sv

#endif // DEVICES_OSCILLOSCOPEDEVICE_HPP
