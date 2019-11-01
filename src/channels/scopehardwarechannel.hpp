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

#ifndef CHANNELS_SCOPEHARDWARECHANNEL_HPP
#define CHANNELS_SCOPEHARDWARECHANNEL_HPP

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

class ScopeHardwareChannel : public HardwareChannel
{
	Q_OBJECT

public:
	/**
	 * A ScopeHardwareChannel does handle interleaved samples from a
	 * scope device.
	 */
	ScopeHardwareChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		set<string> channel_group_names,
		double channel_start_timestamp);

public:
	/**
	 * Add a signal by its quantity, quantity_flags and unit.
	 */
	shared_ptr<data::BaseSignal> add_signal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit) override;

public Q_SLOTS:
	/**
	 * A new frame has started.
	 */
	void on_frame_begin(double timestamp, uint64_t samplerate) override;

private:
	/** The start timestamp of the next signal / of the new scope frame. */
	double next_signal_start_timestamp_;
	/** The actual samplerate of the channel. */
	uint64_t actual_samplerate_;

};

} // namespace channels
} // namespace sv

#endif // CHANNELS_SCOPEHARDWARECHANNEL_HPP
