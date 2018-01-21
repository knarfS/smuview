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

#include "multiplysschannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/device.hpp"

namespace sv {
namespace channels {

MultiplySSChannel::MultiplySSChannel(
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit,
		shared_ptr<data::AnalogSignal> signal_1,
		shared_ptr<data::AnalogSignal> signal_2,
		shared_ptr<devices::Device> parent_device,
		const QString channel_group_name,
		QString channel_name,
		double channel_start_timestamp) :
	MathChannel(sr_quantity, sr_quantity_flags, sr_unit,
		parent_device, channel_group_name, channel_name,
		channel_start_timestamp),
	signal_1_(signal_1),
	signal_2_(signal_2),
	next_signal_1_pos_(0),
	next_signal_2_pos_(0)
{
	assert(signal_1_);
	assert(signal_2_);

	if (signal_1_->digits() >= signal_2_->digits())
		digits_ = signal_1_->digits();
	else
		digits_ = signal_2_->digits();

	connect(signal_1_.get(), SIGNAL(sample_added()),
		this, SLOT(on_sample_added()));
	connect(signal_2_.get(), SIGNAL(sample_added()),
		this, SLOT(on_sample_added()));
}

void MultiplySSChannel::on_sample_added()
{
	// Multiply
	size_t signal_1_sample_count = signal_1_->get_sample_count();
	size_t signal_2_sample_count = signal_2_->get_sample_count();
	while (signal_1_sample_count > next_signal_1_pos_ &&
			signal_2_sample_count > next_signal_2_pos_) {

		data::sample_t sample_1 =
			signal_1_->get_sample(next_signal_1_pos_, false);
		data::sample_t sample_2 =
			signal_2_->get_sample(next_signal_2_pos_, false);

		double time_1 = sample_1.first;
		double time_2 = sample_2.first;
		// TODO: double
		float value = sample_1.second * sample_2.second;

		if (time_1 == time_2)
			push_sample(&value, time_1);
		else
			push_sample(&value, time_1>time_2 ? time_2 : time_1);

		++next_signal_1_pos_;
		++next_signal_2_pos_;
	}
}

} // namespace devices
} // namespace sv
