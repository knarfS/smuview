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

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "hardwarechannel.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::set;
using std::static_pointer_cast;
using std::string;
using std::vector;

namespace sv {
namespace channels {

HardwareChannel::HardwareChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		const set<string> &channel_group_names) :
	BaseChannel(sr_channel, parent_device, channel_group_names)
{
	assert(sr_channel);

	type_ = ChannelType::AnalogChannel;
	name_ = sr_channel_->name();
}

void HardwareChannel::push_samples(const vector<double> &samples,
	double timestamp, uint64_t samplerate, data::Quantity quantity,
	const set<data::QuantityFlag> &quantity_flags, data::Unit unit,
	int sr_digits)
{
	add_signal(quantity, quantity_flags, unit);

	// NOTE: Not implemented in sigrok yet, so using the default for now.
	const int total_digits = data::DefaultTotalDigits;
	static_pointer_cast<data::AnalogTimeSignal>(current_signal_)->push_samples(
		samples, timestamp, samplerate, total_digits, sr_digits);
}

} // namespace channels
} // namespace sv
