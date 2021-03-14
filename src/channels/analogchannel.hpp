/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef CHANNELS_ANALOGCHANNEL_HPP
#define CHANNELS_ANALOGCHANNEL_HPP

#include <memory>
#include <set>
#include <string>

#include <QObject>

#include "src/channels/hardwarechannel.hpp"

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

class AnalogChannel : public HardwareChannel
{
	Q_OBJECT

public:
	/**
	 * A AnalogChannel does handle interleaved samples with timestamps from a
	 * (slow) analog device.
	 */
	AnalogChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		const set<string> &channel_group_names,
		double channel_start_timestamp);

public:
	/**
	 * Close an open frame.
	 */
	void close_frame() override;

	/**
	 * Add one or more interleaved samples with timestamps to the channel
	 */
	void push_interleaved_samples(const float *data,
		const size_t sample_count, const size_t channel_stride,
		const double timestamp, const uint64_t samplerate,
		const uint64_t sample_interval,
		shared_ptr<sigrok::Analog> sr_analog) override;

};

} // namespace channels
} // namespace sv

#endif // CHANNELS_ANALOGCHANNEL_HPP
