/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
#include <memory>
#include <set>
#include <string>

#include <QDebug>

#include "addscchannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::set;
using std::string;

namespace sv {
namespace channels {

AddSCChannel::AddSCChannel(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		shared_ptr<data::AnalogTimeSignal> signal,
		double constant,
		shared_ptr<devices::BaseDevice> parent_device,
		const set<string> &channel_group_names,
		const string &channel_name,
		double channel_start_timestamp) :
	MathChannel(quantity, quantity_flags, unit,
		parent_device, channel_group_names, channel_name,
		channel_start_timestamp),
	signal_(signal),
	constant_(constant),
	next_signal_pos_(0)
{
	assert(signal_);

	digits_ = signal_->digits();
	decimal_places_ = signal_->decimal_places();

	connect(signal_.get(), &data::AnalogTimeSignal::sample_appended,
		this, &AddSCChannel::on_sample_appended);
}

void AddSCChannel::on_sample_appended()
{
	size_t signal_sample_count = signal_->sample_count();
	while (next_signal_pos_ < signal_sample_count) {
		auto sample = signal_->get_sample(next_signal_pos_, false);
		double time = sample.first;
		double value = sample.second + constant_;
		push_sample(value, time);
		++next_signal_pos_;
	}
}

} // namespace channels
} // namespace sv
