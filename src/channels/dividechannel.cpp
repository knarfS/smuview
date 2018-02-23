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

#include "dividechannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/device.hpp"

namespace sv {
namespace channels {

DivideChannel::DivideChannel(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<data::AnalogSignal> dividend_signal,
		shared_ptr<data::AnalogSignal> divisor_signal,
		shared_ptr<devices::Device> parent_device,
		const QString channel_group_name,
		QString channel_name,
		double channel_start_timestamp) :
	MathChannel(quantity, quantity_flags, unit,
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

	if (dividend_signal->decimal_places() >= divisor_signal->decimal_places())
		decimal_places_ = dividend_signal->decimal_places();
	else
		decimal_places_ = divisor_signal->decimal_places();

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

	while (next_dividend_signal_pos_ < dividend_signal_sample_count ||
			next_divisor_signal_pos_ < divisor_signal_sample_count) {

		bool has_sample_1 = false;
		data::sample_t sample_1;
		if (next_dividend_signal_pos_ < dividend_signal_sample_count) {
			sample_1 = dividend_signal_->get_sample(
				next_dividend_signal_pos_, false);
			has_sample_1 = true;
		}

		bool has_sample_2 = false;
		data::sample_t sample_2;
		if (next_divisor_signal_pos_ < divisor_signal_sample_count) {
			sample_2 = divisor_signal_->get_sample(
				next_divisor_signal_pos_, false);
			has_sample_2 = true;
		}

		double time = 0;
		if (has_sample_1 && !has_sample_2 && next_divisor_signal_pos_ == 0) {
			last_dividend_value_ = sample_1.second;
			++next_dividend_signal_pos_;
			continue;
		}
		else if (has_sample_2 && !has_sample_1 && next_dividend_signal_pos_ == 0) {
			last_divisor_value_ = sample_2.second;
			++next_divisor_signal_pos_;
			continue;
		}
		else if (has_sample_1 && !has_sample_2) {
			time = sample_1.first;
			last_dividend_value_ = sample_1.second;
			++next_dividend_signal_pos_;
		}
		else if (has_sample_2 && !has_sample_1) {
			time = sample_2.first;
			last_divisor_value_ = sample_2.second;
			++next_divisor_signal_pos_;
		}
		else if (has_sample_1 && has_sample_2) {
			double time_1 = sample_1.first;
			double time_2 = sample_2.first;
			if (time_1 == time_2) {
				time = time_1;
				last_dividend_value_ = sample_1.second;
				last_divisor_value_ = sample_2.second;
				++next_dividend_signal_pos_;
				++next_divisor_signal_pos_;
			}
			else if (time_1 < time_2) {
				time = time_1;
				last_dividend_value_ = sample_1.second;
				++next_dividend_signal_pos_;
			}
			else if (time_2 < time_1) {
				time = time_2;
				last_divisor_value_ = sample_2.second;
				++next_divisor_signal_pos_;
			}
			else {
				// Something is wrong here...
				qWarning() << "MultiplySSChannel::on_sample_added(): " <<
					"Could not match the two signals!";
			}
		}
		else {
			// Something is wrong here...
			qWarning() << "MultiplySSChannel::on_sample_added(): " <<
				"Could not match the two signals!";
		}

		double value;
		if (last_divisor_value_ == 0) {
			if (last_dividend_value_ > 0)
				// TODO: use infinity() instead?
				value = std::numeric_limits<double>::max();
			else
				// TODO: use -1 * infinity() instead?
				value = std::numeric_limits<double>::lowest();
		}
		else
			value = last_dividend_value_ / last_divisor_value_;
		push_sample(value, time);
	}
}

} // namespace devices
} // namespace sv
