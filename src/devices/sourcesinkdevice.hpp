/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DEVICES_SOURCESINKDEVICE_HPP
#define DEVICES_SOURCESINKDEVICE_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/devices/hardwaredevice.hpp"

using std::forward;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sigrok {
class Context;
class HardwareDevice;
}

namespace sv {
namespace devices {

class SourceSinkDevice : public HardwareDevice
{
	Q_OBJECT

private:
	explicit SourceSinkDevice(const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device);

	bool get_channel_name_suffix(string &channel_suffix,
		const string &channel_name, const vector<string> prefixes,
		bool is_initialized);

public:
	template<typename ...Arg>
	shared_ptr<SourceSinkDevice> static create(Arg&&...arg)
	{
		struct make_shared_enabler : public SourceSinkDevice {
			explicit make_shared_enabler(Arg&&...arg) :
				SourceSinkDevice(forward<Arg>(arg)...)
			{
			}
		};

		return make_shared<make_shared_enabler>(forward<Arg>(arg)...);
	}

protected:
	void init_configurables() override;
	void init_channels() override;

};

} // namespace devices
} // namespace sv

#endif // DEVICES_SOURCESINKDEVICE_HPP
