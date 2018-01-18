/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "hardwarechannel.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"

using std::make_pair;
using std::make_shared;
using std::static_pointer_cast;

namespace sv {
namespace channels {

HardwareChannel::HardwareChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		const QString device_name,
		const QString channel_group_name,
		double channel_start_timestamp) :
	BaseChannel(device_name, channel_group_name, channel_start_timestamp),
	sr_channel_(sr_channel)
{
	assert(sr_channel);

	channel_type_ = ChannelType::AnalogChannel;
	name_ = QString::fromStdString(sr_channel_->name());
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

void HardwareChannel::set_name(QString name)
{
	if (sr_channel_)
		sr_channel_->set_name(name.toUtf8().constData());

	BaseChannel::set_name(name);
}

shared_ptr<sigrok::Channel> HardwareChannel::sr_channel() const
{
	return sr_channel_;
}

shared_ptr<data::BaseSignal> HardwareChannel::init_signal(
	const sigrok::Quantity *sr_quantity,
	vector<const sigrok::QuantityFlag *> sr_quantity_flags,
	const sigrok::Unit *sr_unit)
{
	// TODO: At the moment, only analog channels are supported
	if (sr_channel_->type()->id() != SR_CHANNEL_ANALOG)
		return nullptr;

	shared_ptr<data::AnalogSignal> signal = make_shared<data::AnalogSignal>(
		sr_quantity, sr_quantity_flags, sr_unit,
		device_name_, channel_group_name_, name_,
		channel_start_timestamp_);

	connect(this, SIGNAL(channel_start_timestamp_changed(double)),
			signal.get(), SLOT(on_channel_start_timestamp_changed(double)));

	actual_signal_ = signal;
	quantity_t q_qf = make_pair(sr_quantity, sr_quantity_flags);
	signal_map_.insert(make_pair(q_qf, signal));

	return signal;
}

void HardwareChannel::push_sample_sr_analog(
	void *sample, double timestamp, shared_ptr<sigrok::Analog> sr_analog)
{
	quantity_t q_qf = make_pair(sr_analog->mq(), sr_analog->mq_flags());
	if (signal_map_.count(q_qf) == 0) {
		init_signal(sr_analog->mq(), sr_analog->mq_flags(), sr_analog->unit());
		Q_EMIT signal_changed();
		qWarning() << "HardwareChannel::push_sample_sr_analog(): " << name_ <<
		" - No signal found: " << actual_signal_->name();
	}

	// Number of significant digits after the decimal point if positive, or
	// number of non-significant digits before the decimal point if negative
	// (refers to the value we actually read on the wire).
	qWarning() << "HardwareChannel::push_sample_sr_analog(): sr_analog->digits() = " << sr_analog->digits();
	int digits = 7;
	int decimal_places = -1;
	if (sr_analog->digits() > 0)
		decimal_places = sr_analog->digits();
	else
		digits = -1*sr_analog->digits(); // TODO

	auto signal = static_pointer_cast<data::AnalogSignal>(signal_map_[q_qf]);
	signal->push_sample(sample, timestamp, digits, decimal_places);
}

} // namespace channels
} // namespace sv
