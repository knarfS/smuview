/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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
#include <string>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "hardwarechannel.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/basedevice.hpp"

using std::make_pair;
using std::make_shared;
using std::set;
using std::static_pointer_cast;
using std::string;
using std::unique_ptr;

Q_DECLARE_METATYPE(std::shared_ptr<sv::data::BaseSignal>)

namespace sv {
namespace channels {

HardwareChannel::HardwareChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		set<string> channel_group_names,
		double channel_start_timestamp) :
	BaseChannel(parent_device, channel_group_names, channel_start_timestamp),
	sr_channel_(sr_channel)
{
	assert(sr_channel);

	channel_type_ = ChannelType::AnalogChannel;
	name_ = sr_channel_->name();
}

bool HardwareChannel::enabled() const
{
	return (sr_channel_) ? sr_channel_->enabled() : true;
}

void HardwareChannel::set_enabled(bool value)
{
	if (sr_channel_) {
		sr_channel_->set_enabled(value);
		enabled_changed(value);
	}
}

unsigned int HardwareChannel::index() const
{
	return (sr_channel_) ? sr_channel_->index() : 0;
}

void HardwareChannel::set_name(string name)
{
	if (sr_channel_)
		sr_channel_->set_name(name);

	BaseChannel::set_name(name);
}

shared_ptr<sigrok::Channel> HardwareChannel::sr_channel() const
{
	return sr_channel_;
}

// TODO: Call base
shared_ptr<data::BaseSignal> HardwareChannel::init_signal(
	data::Quantity quantity,
	set<data::QuantityFlag> quantity_flags,
	data::Unit unit)
{
	// TODO: At the moment, only analog channels are supported
	if (sr_channel_->type()->id() != SR_CHANNEL_ANALOG)
		return nullptr;

	shared_ptr<data::AnalogSignal> signal = make_shared<data::AnalogSignal>(
		quantity, quantity_flags, unit,
		shared_from_this(), channel_start_timestamp_);

	connect(this, SIGNAL(channel_start_timestamp_changed(double)),
			signal.get(), SLOT(on_channel_start_timestamp_changed(double)));

	actual_signal_ = signal;
	quantity_t q_qf = make_pair(quantity, quantity_flags);
	signal_map_.insert(make_pair(q_qf, signal));

	Q_EMIT signal_added(signal);

	return signal;
}

void HardwareChannel::push_sample_sr_analog(
	void *sample, double timestamp, shared_ptr<sigrok::Analog> sr_analog)
{
	/*
	 * NOTE: Sometimes the mq is not set (e.g. for the demo driver in
	 *       sigrok 6.0.0) and mq() just throws an exception, without a
	 *       possibility to check if mq is set or not.
	 */
	data::Quantity quantity;
	try {
		quantity = data::datautil::get_quantity(sr_analog->mq());
	}
	catch(sigrok::Error &e) {
		quantity = data::Quantity::Unknown;
	}

	set<data::QuantityFlag> quantity_flags =
		data::datautil::get_quantity_flags(sr_analog->mq_flags());
	quantity_t q_qf = make_pair(quantity, quantity_flags);
	if (signal_map_.count(q_qf) == 0) {
		data::Unit unit = data::datautil::get_unit(sr_analog->unit());
		init_signal(quantity, quantity_flags, unit);
		qWarning() << "HardwareChannel::push_sample_sr_analog(): " <<
			QString::fromStdString(name_) <<
			" - No signal found: " << actual_signal_->name();
	}

	auto signal = static_pointer_cast<data::AnalogSignal>(signal_map_[q_qf]);
	if (signal.get() != actual_signal_.get()) {
		actual_signal_ = signal;
		Q_EMIT signal_changed(actual_signal_);
	}

	// Number of significant digits after the decimal point if positive, or
	// number of non-significant digits before the decimal point if negative
	// (refers to the value we actually read on the wire).
	int digits = 7;
	int decimal_places = -1;
	if (sr_analog->digits() >= 0)
		decimal_places = sr_analog->digits();
	else
		digits = -1 * sr_analog->digits(); // TODO

	signal->push_sample(sample, timestamp,
		sr_analog->unitsize(), digits, decimal_places);
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
	catch(sigrok::Error &e) {
		quantity = data::Quantity::Unknown;
	}

	set<data::QuantityFlag> quantity_flags =
		data::datautil::get_quantity_flags(sr_analog->mq_flags());
	quantity_t q_qf = make_pair(quantity, quantity_flags);
	if (signal_map_.count(q_qf) == 0) {
		data::Unit unit = data::datautil::get_unit(sr_analog->unit());
		init_signal(quantity, quantity_flags, unit);
		qWarning() << "HardwareChannel::push_sample_sr_analog(): " <<
			QString::fromStdString(name_) <<
			" - No signal found: " << actual_signal_->name();
	}

	auto signal = static_pointer_cast<data::AnalogSignal>(signal_map_[q_qf]);
	if (signal.get() != actual_signal_.get()) {
		actual_signal_ = signal;
		Q_EMIT signal_changed(actual_signal_);
	}

	// Number of significant digits after the decimal point if positive, or
	// number of non-significant digits before the decimal point if negative
	// (refers to the value we actually read on the wire).
	int digits = 7;
	int decimal_places = -1;
	if (sr_analog->digits() >= 0)
		decimal_places = sr_analog->digits();
	else
		digits = -1 * sr_analog->digits(); // TODO

	// Deinterleave the samples and add them
	unique_ptr<float[]> deint_data(new float[sample_count]);
	float *deint_data_ptr = deint_data.get();
	for (uint32_t i = 0; i < sample_count; i++) {
		*deint_data_ptr = (float)(*data);
		deint_data_ptr++;
		data += stride;
	}

	signal->push_samples(deint_data.get(), sample_count, timestamp, samplerate,
		sr_analog->unitsize(), digits, decimal_places);
}

} // namespace channels
} // namespace sv
