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

#include <cassert>
#include <memory>
#include <set>

#include <QDebug>

#include "mathchannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/device.hpp"

using std::make_pair;
using std::make_shared;
using std::set;
using std::static_pointer_cast;

namespace sv {
namespace channels {

MathChannel::MathChannel(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<devices::Device> parent_device,
		const QString channel_group_name,
		QString channel_name,
		double channel_start_timestamp) :
	BaseChannel(parent_device, channel_group_name, channel_start_timestamp),
	digits_(8),
	decimal_places_(-1),
	quantity_(quantity),
	quantity_flags_(quantity_flags),
	unit_(unit)
{
	assert(quantity_);
	//assert(quantity_flags_);
	assert(unit_);

	channel_type_ = ChannelType::MathChannel;
	has_fixed_signal_ = true;
	name_ = channel_name;
}

unsigned int MathChannel::index() const
{
	return 1000;
}

shared_ptr<data::BaseSignal> MathChannel::init_signal()
{
	// TODO: At the moment, only analog channels are supported
	shared_ptr<data::AnalogSignal> signal = make_shared<data::AnalogSignal>(
		quantity_,quantity_flags_, unit_,
		shared_from_this(), channel_start_timestamp_);

	connect(this, SIGNAL(channel_start_timestamp_changed(double)),
		signal.get(), SLOT(on_channel_start_timestamp_changed(double)));

	actual_signal_ = signal;
	quantity_t q_qf = make_pair(quantity_, quantity_flags_);
	signal_map_.insert(make_pair(q_qf, signal));

	Q_EMIT signal_added(signal);

	return signal;
}

void MathChannel::push_sample(double sample, double timestamp)
{
	auto signal = static_pointer_cast<data::AnalogSignal>(actual_signal_);
	signal->push_sample(&sample, timestamp,
		size_of_double_, digits_, decimal_places_);
}

} // namespace devices
} // namespace sv
