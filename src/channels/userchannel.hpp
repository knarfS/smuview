/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2019 Frank Stettner <frank-stettner@gmx.net>
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

enum class MathChannelType {
		MultiplyChannel,
		DivideChannel,
		IntegrateChannel
};

class UserChannel : public BaseChannel
{
	Q_OBJECT

public:
	UserChannel(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<devices::BaseDevice> parent_device,
		set<string> channel_group_names,
		string channel_name,
		double channel_start_timestamp);

public:
	/**
	 * Gets the index number of this channel
	 */
	unsigned int index() const;

	/**
	 * Inits a signal
	 *
	 * TODO
	 */
	/*
	shared_ptr<data::BaseSignal> init_signal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,);
	*/

	/**
	 * Inits a signal.
	 *
	 * Must be called after instanziation of the object, because of the use of
	 * shared_from_this() in here! init_signal().
	 *
	 * TODO: Move to base
	 * TODO: Reinmplement, so that init_signal() can be called from ctor
	 */
	shared_ptr<data::BaseSignal> init_signal();

protected:
	/**
	 * Add a single sample with timestamp to the channel/signal
	 */
	void push_sample(double sample, double timestamp);

	int digits_;
	int decimal_places_;
	data::Quantity quantity_;
	set<data::QuantityFlag> quantity_flags_;
	data::Unit unit_;

	static const size_t size_of_double_ = sizeof(double);

};

} // namespace channels
} // namespace sv

#endif // CHANNELS_USERCHANNEL_HPP
