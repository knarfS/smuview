/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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

#include "scopechannel.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/data/analogscopesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::make_pair;
using std::make_shared;
using std::set;
using std::static_pointer_cast;
using std::string;
using std::unique_ptr;
using sv::data::measured_quantity_t;

namespace sv {
namespace channels {

ScopeChannel::ScopeChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		const set<string> &channel_group_names,
		double channel_start_timestamp) :
	HardwareChannel(sr_channel, parent_device, channel_group_names,
		channel_start_timestamp)
{
	type_ = ChannelType::ScopeChannel;
}

void ScopeChannel::close_frame()
{
	if (actual_signal_ == nullptr)
		return;

	static_pointer_cast<data::AnalogScopeSignal>(actual_signal_)->
		complete_actual_segment();
}

void ScopeChannel::push_interleaved_samples(const float *data,
	const size_t sample_count, const size_t channel_stride,
	const double timestamp, const uint64_t samplerate,
	const uint64_t sample_interval,
	shared_ptr<sigrok::Analog> sr_analog)
{
	//(void)timestamp;
	//(void)samplerate;
	(void)sample_interval;

	//lock_guard<recursive_mutex> lock(mutex_);

	/*
	 * NOTE: Sometimes the mq is not set (e.g. for the demo driver in
	 *       sigrok 6.0.0) and mq() just throws an exception, without a
	 *       possibility to check if mq is set or not.
	 * /
	data::Quantity quantity;
	try {
		quantity = data::datautil::get_quantity(sr_analog->mq());
	}
	catch(sigrok::Error &e) {
		quantity = data::Quantity::Unknown;
	}
	set<data::QuantityFlag> quantity_flags =
		data::datautil::get_quantity_flags(sr_analog->mq_flags());

	if (!actual_signal_ || actual_signal_->quantity() != quantity ||
		actual_signal_->quantity_flags() != quantity_flags) {

		/ * actual_signal_ not set or doesn't match the mq/mqf * /
		measured_quantity_t mq = make_pair(quantity, quantity_flags);
		size_t signals_count = signal_map_.count(mq);
		if (signals_count == 0) {
			data::Unit unit = data::datautil::get_unit(sr_analog->unit());
			add_signal(quantity, quantity_flags, unit);
			qWarning() << "HardwareChannel::push_interleaved_samples(): " <<
				display_name() << " - No signal found: " <<
				actual_signal_->display_name();
		}
		else if (signals_count > 1) {
			throw ("More than one signal found for " + name());
		}

		actual_signal_ = signal_map_[mq][0];
		Q_EMIT signal_changed(actual_signal_);
	}
	*/

	/*
	 * Number of significant digits after the decimal point if positive, or
	 * number of non-significant digits before the decimal point if negative
	 * (refers to the value we actually read on the wire).
	 */
	int digits = 7;
	int decimal_places = -1;
	if (sr_analog->digits() >= 0)
		decimal_places = sr_analog->digits();
	else
		digits = -1 * sr_analog->digits(); // TODO

	// TODO: Move deinterleaving to HardwareDevice::feed_in_analog()
	// Deinterleave the samples and add them
	unique_ptr<float[]> deint_data(new float[sample_count]);
	float *deint_data_ptr = deint_data.get();
	for (size_t i = 0; i < sample_count; i++) {
		*deint_data_ptr = (float)(*data);
		deint_data_ptr++;
		data += channel_stride;
	}

	if (!actual_signal_) {
		qWarning() << "ScopeChannel::push_interleaved_samples(): " <<
			display_name() << " - No actual_signal_ found!";

		/* TODO: integrate into BaseChannel::add_signal() !! */

		auto signal = make_shared<data::AnalogScopeSignal>(
			data::Quantity::Voltage, set<data::QuantityFlag>(),
			data::Unit::Volt, shared_from_this(), timestamp, samplerate  /*, custom_name*/);

		connect(this, SIGNAL(channel_start_timestamp_changed(double)),
			signal.get(), SLOT(on_channel_start_timestamp_changed(double)));

		measured_quantity_t mq = make_pair(
			signal->quantity(), signal->quantity_flags());
		if (signal_map_.count(mq) > 0) {
			signal_map_[mq].push_back(signal);
		}
		else {
			signal_map_.insert(
				make_pair(mq, vector<shared_ptr<data::BaseSignal>> { signal }));
		}

		actual_signal_ = signal;
		Q_EMIT signal_added(signal);
	}

	// TODO: remove cast
	static_pointer_cast<data::AnalogScopeSignal>(actual_signal_)->push_samples(
		deint_data.get(), sample_count, timestamp, samplerate,
		size_of_float_ /*sr_analog->unitsize() TODO */, digits, decimal_places);
}

} // namespace channels
} // namespace sv
