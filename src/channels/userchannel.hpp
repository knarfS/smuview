/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2022 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef CHANNELS_USERCHANNEL_HPP
#define CHANNELS_USERCHANNEL_HPP

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <QObject>

#include "src/channels/basechannel.hpp"
#include "src/data/datautil.hpp"

using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {

namespace data {
class BaseSignal;
}

namespace devices {
class BaseDevice;
}

namespace channels {

class UserChannel : public BaseChannel
{
	Q_OBJECT

public:
	UserChannel(
		const string &channel_name,
		const set<string> &channel_group_names,
		shared_ptr<devices::BaseDevice> parent_device,
		double channel_start_timestamp);

	/**
	 * Add a single sample with timestamp to the channel/signal
	 * TODO: Move to base?
	 */
	void push_sample(double sample, double timestamp,
		data::Quantity quantity, set<data::QuantityFlag> quantity_flags,
		data::Unit unit, int total_digits, int sr_digits);

};

} // namespace channels
} // namespace sv

#endif // CHANNELS_USERCHANNEL_HPP
