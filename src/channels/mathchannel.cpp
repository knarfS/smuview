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

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "mathchannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/device.hpp"

using std::make_pair;
using std::make_shared;
using std::static_pointer_cast;

namespace sv {
namespace channels {

MathChannel::MathChannel(
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit,
		shared_ptr<devices::Device> parent_device,
		const QString channel_group_name,
		QString channel_name,
		double channel_start_timestamp) :
	BaseChannel(parent_device, channel_group_name, channel_start_timestamp),
	digits_(8),
	decimal_places_(-1),
	sr_quantity_(sr_quantity),
	sr_quantity_flags_(sr_quantity_flags),
	sr_unit_(sr_unit)
{
	assert(sr_quantity_);
	//assert(sr_quantity_flags_);
	assert(sr_unit_);

	channel_type_ = ChannelType::MathChannel;
	has_fixed_signal_ = true;
	name_ = channel_name;

	init_signal(sr_quantity_, sr_quantity_flags_, sr_unit_);
}

unsigned int MathChannel::index() const
{
	return 1000;
}

// TODO: Move to base
shared_ptr<data::BaseSignal> MathChannel::init_signal(
	const sigrok::Quantity *sr_quantity,
	vector<const sigrok::QuantityFlag *> sr_quantity_flags,
	const sigrok::Unit *sr_unit)
{
	// TODO: use class sr_quantity_, sr_quantity_flags_, sr_unit_
	// and name init_signal() for math channles

	// TODO: At the moment, only analog channels are supported
	shared_ptr<data::AnalogSignal> signal = make_shared<data::AnalogSignal>(
		sr_quantity, sr_quantity_flags, sr_unit,
		shared_from_this(), channel_start_timestamp_);

	connect(this, SIGNAL(channel_start_timestamp_changed(double)),
		signal.get(), SLOT(on_channel_start_timestamp_changed(double)));

	actual_signal_ = signal;
	quantity_t q_qf = make_pair(sr_quantity, sr_quantity_flags);
	signal_map_.insert(make_pair(q_qf, signal));

	Q_EMIT signal_added(signal);

	return signal;
}

void MathChannel::push_sample(void *sample, double timestamp)
{
	auto signal = static_pointer_cast<data::AnalogSignal>(actual_signal_);
	signal->push_sample(sample, timestamp, digits_, decimal_places_);
}

} // namespace devices
} // namespace sv
