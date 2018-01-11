/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DEVICES_SOURCESINKDEVICE_HPP
#define DEVICES_SOURCESINKDEVICE_HPP

#include <memory>
#include <vector>

#include "src/devices/hardwaredevice.hpp"

using std::shared_ptr;
using std::vector;

namespace sigrok {
class Channel;
class Context;
class HardwareDevice;
class Meta;
}

namespace sv {

namespace data {
class AnalogSignal;
}

namespace devices {

class SourceSinkDevice final : public HardwareDevice
{
	Q_OBJECT

public:
	explicit SourceSinkDevice(const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device);

	~SourceSinkDevice();

protected:
	void feed_in_meta(shared_ptr<sigrok::Meta> sr_meta);
	void init_signal(
		shared_ptr<sigrok::Channel> sr_channel,
		QString channel_group_name,
		shared_ptr<vector<double>> common_time_data);

private:

};

} // namespace devices
} // namespace sv

#endif // DEVICES_SOURCESINKDEVICE_HPP
