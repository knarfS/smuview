/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2026 Frank Stettner <frank-stettner@gmx.net>
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

#include "userchannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::set;
using std::static_pointer_cast;
using std::string;

namespace sv {
namespace channels {

UserChannel::UserChannel(
		const string &channel_name,
		const set<string> &channel_group_names,
		shared_ptr<devices::BaseDevice> parent_device) :
	BaseChannel(nullptr, parent_device, channel_group_names)
{
	name_ = channel_name;
	type_ = ChannelType::UserChannel;
	index_ = parent_device->next_channel_index();
	fixed_signal_ = false;

	// TODO: Move to device, then we can remove shared_from_this
	if (devices::deviceutil::is_user_device(parent_device_->type())) {
		auto sr_udev = static_pointer_cast<sigrok::UserDevice>(
			parent_device_->sr_device());
		sr_channel_ = sr_udev->add_channel(
			index_, sigrok::ChannelType::ANALOG, name_);
	}
}

void UserChannel::push_sample(double sample, double timestamp,
	data::Quantity quantity, set<data::QuantityFlag> quantity_flags,
	data::Unit unit, int total_digits, int sr_digits)
{
	add_signal(quantity, quantity_flags, unit);
	static_pointer_cast<data::AnalogTimeSignal>(current_signal_)->push_sample(
		sample, timestamp, total_digits, sr_digits);
}

} // namespace channels
} // namespace sv
