/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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
#include <memory>
#include <set>
#include <string>

#include <QDebug>

#include "userchannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::make_pair;
using std::make_shared;
using std::set;
using std::static_pointer_cast;
using std::string;

namespace sv {
namespace channels {

UserChannel::UserChannel(
		string channel_name,
		set<string> channel_group_names,
		shared_ptr<devices::BaseDevice> parent_device,
		double channel_start_timestamp) :
	BaseChannel(nullptr, parent_device, channel_group_names,
		channel_start_timestamp)
{
	name_ = channel_name;
	channel_type_ = ChannelType::UserChannel;
	channel_index_ = parent_device->next_channel_index();
	fixed_signal_ = false;

	if (parent_device_->type() == devices::DeviceType::UserDevice) {
		auto sr_udev = static_pointer_cast<sigrok::UserDevice>(
			parent_device_->sr_device());
		sr_channel_ = sr_udev->add_channel(
			channel_index_, sigrok::ChannelType::ANALOG, name_);
	}
}

void UserChannel::push_sample(double sample, double timestamp,
	data::Quantity quantity, set<data::QuantityFlag> quantity_flags,
	data::Unit unit, int digits, int decimal_places)
{
	if (!actual_signal_ || actual_signal_->quantity() != quantity ||
		actual_signal_->quantity_flags() != quantity_flags) {

		measured_quantity_t mq = make_pair(quantity, quantity_flags);
		size_t signals_count = signal_map_.count(mq);
		if (signals_count == 0) {
			actual_signal_ = add_signal(quantity, quantity_flags, unit);
			qWarning() << "UserChannel::push_sample(): " << display_name() <<
				" - No signal found: " << actual_signal_->display_name();
		}
		else if (signals_count > 1) {
			actual_signal_ = signal_map_[mq][0];
			qWarning() << "UserChannel::push_sample(): " << display_name() <<
				" - More than one signal found, using first found signal: " <<
				actual_signal_->display_name();
		}
		Q_EMIT signal_changed(actual_signal_);
	}

	static_pointer_cast<data::AnalogTimeSignal>(actual_signal_)->push_sample(
		&sample, timestamp, size_of_double_, digits, decimal_places);
}

} // namespace devices
} // namespace sv
