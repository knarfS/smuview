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

#include "mathchannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::set;
using std::static_pointer_cast;
using std::string;
using sv::data::measured_quantity_t;

namespace sv {
namespace channels {

MathChannel::MathChannel(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<devices::BaseDevice> parent_device,
		set<string> channel_group_names,
		string channel_name,
		double channel_start_timestamp) :
	BaseChannel(nullptr, parent_device, channel_group_names,
			channel_start_timestamp),
	digits_(7),
	decimal_places_(-1),
	quantity_(quantity),
	quantity_flags_(quantity_flags),
	unit_(unit)
{
	name_ = channel_name;
	channel_type_ = ChannelType::MathChannel;
	channel_index_ = parent_device->next_channel_index();
	fixed_signal_ = true;

	if (parent_device_->type() == devices::DeviceType::UserDevice) {
		auto sr_udev = static_pointer_cast<sigrok::UserDevice>(
			parent_device_->sr_device());
		sr_channel_ = sr_udev->add_channel(
			channel_index_, sigrok::ChannelType::ANALOG, name_);
	}

	/*
	 * TODO: Remove shared_from_this() / (channel pointer in signal), so that
	 *       "add_signal()" can be called from MathChannel ctor.
	 *       But are the signals channel_added() and signal_added() in
	 *       the correct order then?
	add_signal(quantity_, quantity_flags_, unit_);
	 */
}

data::Quantity MathChannel::quantity()
{
	return quantity_;
}

set<data::QuantityFlag> MathChannel::quantity_flags()
{
	return quantity_flags_;
}

data::Unit MathChannel::unit()
{
	return unit_;
}

void MathChannel::push_sample(double sample, double timestamp)
{
	auto signal = static_pointer_cast<data::AnalogTimeSignal>(actual_signal_);
	signal->push_sample(&sample, timestamp,
		size_of_double_, digits_, decimal_places_);
}

} // namespace channels
} // namespace sv
