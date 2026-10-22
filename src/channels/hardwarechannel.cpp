/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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
#include <utility>
#include <vector>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "hardwarechannel.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::make_pair;
using std::set;
using std::static_pointer_cast;
using std::string;
using std::vector;
using sv::data::measured_quantity_t;

namespace sv {
namespace channels {

HardwareChannel::HardwareChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		const set<string> &channel_group_names,
		double channel_start_timestamp) :
	BaseChannel(sr_channel, parent_device, channel_group_names,
		channel_start_timestamp)
{
	assert(sr_channel);

	type_ = ChannelType::AnalogChannel;
	name_ = sr_channel_->name();
}

void HardwareChannel::push_samples(const vector<double> &samples,
	double timestamp, uint64_t samplerate, data::Quantity quantity,
	const set<data::QuantityFlag> &quantity_flags, data::Unit unit,
	int sr_digits)
{
	//lock_guard<recursive_mutex> lock(mutex_);

	if (!actual_signal_ || actual_signal_->quantity() != quantity ||
		actual_signal_->quantity_flags() != quantity_flags) {

		/* actual_signal_ not set or doesn't match the mq/mqf */
		measured_quantity_t mq = make_pair(quantity, quantity_flags);
		size_t signals_count = signal_map_.count(mq);
		if (signals_count == 0) {
			add_signal(quantity, quantity_flags, unit);
			qWarning() << "HardwareChannel::push_samples(): "
				<< display_name()
				<< " - Signal was not found and was therefore created: "
				<< actual_signal_->display_name();
		}
		else if (signals_count > 1) {
			throw ("More than one signal found for " + name());
		}

		actual_signal_ = signal_map_[mq][0];
		Q_EMIT signal_changed(actual_signal_);
	}

	// NOTE: Not implemented in sigrok yet, so using the default for now.
	const int total_digits = data::DefaultTotalDigits;

	static_pointer_cast<data::AnalogTimeSignal>(actual_signal_)->push_samples(
		samples, timestamp, samplerate, total_digits, sr_digits);
}

} // namespace channels
} // namespace sv
