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

#include <cassert>
#include <memory>
#include <set>
#include <string>

#include <QDebug>

#include "mathchannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/channelspec.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/deviceref.hpp"

using std::set;
using std::static_pointer_cast;
using std::string;
using sv::data::measured_quantity_t;

namespace sv {
namespace channels {

MathChannel::MathChannel(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		const channels::ChannelSpec &channel_spec,
		const devices::DeviceRef &parent_device) :
	BaseChannel(channel_spec, parent_device),
	total_digits_(data::DefaultTotalDigits),
	sr_digits_(data::DefaultSRDigits),
	quantity_(quantity),
	quantity_flags_(quantity_flags),
	unit_(unit)
{
	type_ = ChannelType::MathChannel;
	fixed_signal_ = true;

	/*
	 * TODO: Are the signals channel_added() and signal_added() in the
	 *       correct order?
	 */
	add_signal(quantity_, quantity_flags_, unit_);
}

void MathChannel::push_sample(double sample, double timestamp)
{
	const auto signal =
		static_pointer_cast<data::AnalogTimeSignal>(current_signal());
	if (!signal)
		return;

	signal->push_sample(sample, timestamp, total_digits_, sr_digits_);
}

} // namespace channels
} // namespace sv
