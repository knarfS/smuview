/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef CHANNELS_HARDWARECHANNEL_HPP
#define CHANNELS_HARDWARECHANNEL_HPP

#include <memory>
#include <set>
#include <string>

#include <QObject>

#include "src/channels/basechannel.hpp"

using std::set;
using std::shared_ptr;
using std::string;

namespace sigrok {
class Analog;
class Channel;
}

namespace sv {

namespace devices {
class BaseDevice;
}

namespace channels {

class HardwareChannel : public BaseChannel
{
	Q_OBJECT

public:
	/**
	 * A HardwareChannel does handle interleaved samples from a
	 * (hardware) device.
	 */
	HardwareChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		set<string> channel_group_names,
		double channel_start_timestamp);

public:
	/**
	 * Add one or more interleaved samples with timestamps to the channel
	 */
	void push_interleaved_samples(const float *data, size_t sample_count,
		size_t stride, double timestamp, uint64_t samplerate,
		shared_ptr<sigrok::Analog> sr_analog);

};

} // namespace channels
} // namespace sv

#endif // CHANNELS_HARDWARECHANNEL_HPP
