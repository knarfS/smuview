/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <assert.h>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "mathchannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"

using std::make_pair;
using std::make_shared;

namespace sv {
namespace channels {

MathChannel::MathChannel(
		QString channel_group_name, double channel_start_timestamp) :
	BaseChannel(channel_group_name, channel_start_timestamp)//,
	//channel_type_(ChannelType::MathChannel),
	//name_("MatchChannel"),
	//internal_name_("MatchChannel")
{
}

unsigned int MathChannel::index() const
{
	return 1000;
}

shared_ptr<data::BaseSignal> MathChannel::init_signal(
	const sigrok::Quantity *sr_quantity,
	vector<const sigrok::QuantityFlag *> sr_quantity_flags,
	const sigrok::Unit *sr_unit)
{
	// TODO: At the moment, only analog channels are supported
	shared_ptr<data::AnalogSignal> signal = make_shared<data::AnalogSignal>(
		sr_quantity, sr_quantity_flags, sr_unit,
		internal_name_, channel_group_name_, channel_start_timestamp_);

	connect(this, SIGNAL(channel_start_timestamp_changed(double)),
			signal.get(), SLOT(on_channel_start_timestamp_changed(double)));

	actual_signal_ = signal;
	quantity_t q_qf = make_pair(sr_quantity, sr_quantity_flags);
	signal_map_.insert(make_pair(q_qf, signal));

	return signal;
}

} // namespace devices
} // namespace sv
