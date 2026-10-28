/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef CHANNELS_HARDWARECHANNEL_HPP
#define CHANNELS_HARDWARECHANNEL_HPP

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <QObject>

#include "src/channels/basechannel.hpp"

using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sigrok {
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
		const set<string> &channel_group_names);

public:
	/**
	 * Add samples with timestamps to the channel
	 */
	void push_samples(const vector<double> &samples, double timestamp,
		uint64_t samplerate, data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags, data::Unit unit,
		int sr_digits);

};

} // namespace channels
} // namespace sv

#endif // CHANNELS_HARDWARECHANNEL_HPP
