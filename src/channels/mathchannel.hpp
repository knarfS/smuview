/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef CHANNELS_MATHCHANNEL_HPP
#define CHANNELS_MATHCHANNEL_HPP

#include <memory>
#include <vector>

#include <QObject>

#include "src/channels/basechannel.hpp"

using std::shared_ptr;
using std::vector;

namespace sigrok {
class Quantity;
class QuantityFlag;
class Unit;
}

namespace sv {

namespace data {
class BaseSignal;
}

namespace channels {

enum class MathChannelType {
		MultiplyChannel,
		DivideChannel,
		IntegrateChannel
};

class MathChannel : public BaseChannel
{
	Q_OBJECT

public:
	MathChannel(
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit,
		const QString device_name,
		const QString channel_group_name,
		QString channel_name,
		double channel_start_timestamp);

public:
	/**
	 * Gets the index number of this channel
	 */
	unsigned int index() const;

	/**
	 * Inits a signal
	 */
	shared_ptr<data::BaseSignal> init_signal(
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit);

protected:
	/**
	 * Add a single sample with timestamp to the channel/signal
	 */
	void push_sample(void *sample, double timestamp);

	int digits_;
	int decimal_places_;
	const sigrok::Quantity *sr_quantity_;
	vector<const sigrok::QuantityFlag *> sr_quantity_flags_;
	const sigrok::Unit *sr_unit_;

};

} // namespace channels
} // namespace sv

#endif // CHANNELS_MATHCHANNEL_HPP
