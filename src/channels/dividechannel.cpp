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

#include "dividechannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/device.hpp"

namespace sv {
namespace channels {

DivideChannel::DivideChannel(
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit,
		shared_ptr<data::AnalogSignal> dividend_signal,
		shared_ptr<data::AnalogSignal> divisor_signal,
		shared_ptr<devices::Device> parent_device,
		const QString channel_group_name,
		QString channel_name,
		double channel_start_timestamp) :
	MathChannel(sr_quantity, sr_quantity_flags, sr_unit,
		parent_device, channel_group_name, channel_name,
		channel_start_timestamp),
	dividend_signal_(dividend_signal),
	divisor_signal_(divisor_signal),
	next_dividend_signal_pos_(0),
	next_divisor_signal_pos_(0)
{
	assert(dividend_signal_);
	assert(divisor_signal_);

	if (dividend_signal->digits() >= divisor_signal->digits())
		digits_ = dividend_signal->digits();
	else
		digits_ = divisor_signal->digits();

	connect(dividend_signal_.get(), SIGNAL(sample_added()),
		this, SLOT(on_sample_added()));
	connect(divisor_signal_.get(), SIGNAL(sample_added()),
		this, SLOT(on_sample_added()));
}

void DivideChannel::on_sample_added()
{
	// Divide
	size_t dividend_signal_sample_count = dividend_signal_->get_sample_count();
	size_t divisor_signal_sample_count = divisor_signal_->get_sample_count();
	while (next_dividend_signal_pos_ < dividend_signal_sample_count  &&
			next_divisor_signal_pos_ < divisor_signal_sample_count) {

		data::sample_t dividend_sample =
			dividend_signal_->get_sample(next_dividend_signal_pos_, false);
		data::sample_t divisor_sample =
			divisor_signal_->get_sample(next_divisor_signal_pos_, false);

		double time1 = dividend_sample.first;
		double time2 = divisor_sample.first;
		double value;
		if (divisor_sample.second == 0) {
			if (dividend_sample.second > 0)
				// TODO: use infinity() instead?
				value = std::numeric_limits<double>::max();
			else
				// TODO: use -1 * infinity() instead?
				value = std::numeric_limits<double>::lowest();
		}
		else
			value = dividend_sample.second / divisor_sample.second;

		push_sample(value, time1>time2 ? time2 : time1);

		++next_dividend_signal_pos_;
		++next_divisor_signal_pos_;
	}
}

} // namespace devices
} // namespace sv
