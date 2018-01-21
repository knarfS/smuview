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

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "multiplysfchannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/device.hpp"

namespace sv {
namespace channels {

MultiplySFChannel::MultiplySFChannel(
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit,
		shared_ptr<data::AnalogSignal> signal,
		double factor,
		shared_ptr<devices::Device> parent_device,
		const QString channel_group_name,
		QString channel_name,
		double channel_start_timestamp) :
	MathChannel(sr_quantity, sr_quantity_flags, sr_unit,
		parent_device, channel_group_name, channel_name,
		channel_start_timestamp),
	signal_(signal),
	factor_(factor),
	next_signal_pos_(0)
{
	assert(signal_);

	digits_ = signal_->digits();

	connect(signal_.get(), SIGNAL(sample_added()),
		this, SLOT(on_sample_added()));
}

void MultiplySFChannel::on_sample_added()
{
	size_t signal_sample_count = signal_->get_sample_count();
	while (signal_sample_count > next_signal_pos_) {
		data::sample_t sample = signal_->get_sample(next_signal_pos_, false);
		double time = sample.first;
		// TODO: double
		float value = sample.second * factor_;
		push_sample(&value, time);
		++next_signal_pos_;
	}
}

} // namespace devices
} // namespace sv
