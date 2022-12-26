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

#ifndef CHANNELS_MATHCHANNEL_HPP
#define CHANNELS_MATHCHANNEL_HPP

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

class MathChannel : public BaseChannel
{
	Q_OBJECT

public:
	MathChannel(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		shared_ptr<devices::BaseDevice> parent_device,
		const set<string> &channel_group_names,
		const string &channel_name,
		double channel_start_timestamp);

	/**
	 * Get the quantity of the math channel.
	 * TODO: remove when add_signal() is calles in the MathChannel ctor
	 */
	data::Quantity quantity();

	/**
	 * Get the quantity flags of the math channel.
	 * TODO: remove when add_signal() is calles in the MathChannel ctor
	 */
	set<data::QuantityFlag> quantity_flags();

	/**
	 * Get the unit of the math channel
	 * TODO: remove when add_signal() is calles in the MathChannel ctor
	 */
	data::Unit unit();

protected:
	/**
	 * Add a single sample with timestamp to the channel/signal
	 */
	void push_sample(double sample, double timestamp);

	int total_digits_;
	int sr_digits_;
	data::Quantity quantity_;
	set<data::QuantityFlag> quantity_flags_;
	data::Unit unit_;

};

} // namespace channels
} // namespace sv

#endif // CHANNELS_MATHCHANNEL_HPP
