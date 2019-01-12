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

#include <cassert>
#include <memory>
#include <set>

#include <QDebug>

#include "multiplysschannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/userchannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

namespace sv {
namespace channels {

MultiplySSChannel::MultiplySSChannel(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<data::AnalogSignal> signal_1,
		shared_ptr<data::AnalogSignal> signal_2,
		shared_ptr<devices::BaseDevice> parent_device,
		const QString channel_group_name,
		QString channel_name,
		double channel_start_timestamp) :
	UserChannel(quantity, quantity_flags, unit,
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

	if (signal_1_->decimal_places() >= signal_2_->decimal_places())
		decimal_places_ = signal_1_->decimal_places();
	else
		decimal_places_ = signal_2_->decimal_places();

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

	while (next_signal_1_pos_ < signal_1_sample_count ||
			next_signal_2_pos_ < signal_2_sample_count) {

		bool has_sample_1 = false;
		data::sample_t sample_1;
		if (next_signal_1_pos_ < signal_1_sample_count) {
			sample_1 = signal_1_->get_sample(next_signal_1_pos_, false);
			has_sample_1 = true;
		}

		bool has_sample_2 = false;
		data::sample_t sample_2;
		if (next_signal_2_pos_ < signal_2_sample_count) {
			sample_2 = signal_2_->get_sample(next_signal_2_pos_, false);
			has_sample_2 = true;
		}

		double time = 0;
		if (has_sample_1 && !has_sample_2 && next_signal_2_pos_ == 0) {
			last_signal_1_value_ = sample_1.second;
			++next_signal_1_pos_;
			continue;
		}
		else if (has_sample_2 && !has_sample_1 && next_signal_1_pos_ == 0) {
			last_signal_2_value_ = sample_2.second;
			++next_signal_2_pos_;
			continue;
		}
		else if (has_sample_1 && !has_sample_2) {
			time = sample_1.first;
			last_signal_1_value_ = sample_1.second;
			++next_signal_1_pos_;
		}
		else if (has_sample_2 && !has_sample_1) {
			time = sample_2.first;
			last_signal_2_value_ = sample_2.second;
			++next_signal_2_pos_;
		}
		else if (has_sample_1 && has_sample_2) {
			double time_1 = sample_1.first;
			double time_2 = sample_2.first;
			if (time_1 == time_2) {
				time = time_1;
				last_signal_1_value_ = sample_1.second;
				last_signal_2_value_ = sample_2.second;
				++next_signal_1_pos_;
				++next_signal_2_pos_;
			}
			else if (time_1 < time_2) {
				time = time_1;
				last_signal_1_value_ = sample_1.second;
				++next_signal_1_pos_;
			}
			else if (time_2 < time_1) {
				time = time_2;
				last_signal_2_value_ = sample_2.second;
				++next_signal_2_pos_;
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

		push_sample(last_signal_1_value_ * last_signal_2_value_, time);
	}
}

} // namespace devices
} // namespace sv
