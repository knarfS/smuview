/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2022 Frank Stettner <frank-stettner@gmx.net>
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
using std::unique_ptr;
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

void HardwareChannel::push_interleaved_samples(const float *data,
	size_t sample_count, size_t stride, double timestamp, uint64_t samplerate,
	shared_ptr<sigrok::Analog> sr_analog)
{
	//lock_guard<recursive_mutex> lock(mutex_);

	/*
	 * NOTE: Sometimes the mq is not set (e.g. for the demo driver in
	 *       sigrok 6.0.0) and mq() just throws an exception, without a
	 *       possibility to check if mq is set or not.
	 */
	data::Quantity quantity;
	try {
		quantity = data::datautil::get_quantity(sr_analog->mq());
	}
	catch (const sigrok::Error &e) {
		quantity = data::Quantity::Unknown;
	}
	set<data::QuantityFlag> quantity_flags =
		data::datautil::get_quantity_flags(sr_analog->mq_flags());

	if (!actual_signal_ || actual_signal_->quantity() != quantity ||
		actual_signal_->quantity_flags() != quantity_flags) {

		/* actual_signal_ not set or doesn't match the mq/mqf */
		measured_quantity_t mq = make_pair(quantity, quantity_flags);
		size_t signals_count = signal_map_.count(mq);
		if (signals_count == 0) {
			data::Unit unit = data::datautil::get_unit(sr_analog->unit());
			add_signal(quantity, quantity_flags, unit);
			qWarning() << "HardwareChannel::push_sample_sr_analog(): "
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

	// Deinterleave the samples and add them
	unique_ptr<float[]> deint_data(new float[sample_count]);
	float *deint_data_ptr = deint_data.get();
	for (size_t i = 0; i < sample_count; i++) {
		*deint_data_ptr = (float)(*data);
		deint_data_ptr++;
		data += stride;
	}

	// NOTE: Not implementet in sigrok yet, so using the default for now.
	const int total_digits = data::DefaultTotalDigits;

	static_pointer_cast<data::AnalogTimeSignal>(actual_signal_)->push_samples(
		deint_data.get(), sample_count, timestamp, samplerate,
		sr_analog->unitsize(), total_digits, sr_analog->digits());
}

} // namespace channels
} // namespace sv
