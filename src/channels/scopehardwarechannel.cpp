/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>
#include <set>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "scopehardwarechannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/data/analogscopesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::make_shared;
using std::set;

namespace sv {
namespace channels {

ScopeHardwareChannel::ScopeHardwareChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		set<string> channel_group_names,
		double channel_start_timestamp) :
	HardwareChannel(sr_channel, parent_device, channel_group_names,
		channel_start_timestamp)
{
	channel_type_ = ChannelType::ScopeChannel;

	qWarning() << "ScopeHardwareChannel::ScopeHardwareChannel(): name = " << display_name();
}

shared_ptr<data::BaseSignal> ScopeHardwareChannel::add_signal(
	data::Quantity quantity,
	set<data::QuantityFlag> quantity_flags,
	data::Unit unit)
{
	/*
	 * TODO: Remove shared_from_this() / (channel pointer in signal), so that
	 *       "add_signal()" can be called from MathChannel ctor.
	 */
	auto signal = make_shared<data::AnalogScopeSignal>(
		quantity, quantity_flags, unit,
		shared_from_this(), next_signal_start_timestamp_, actual_samplerate_);

	qWarning() << "ScopeHardwareChannel::add_signal(): samplerate = " << actual_samplerate_;
	BaseChannel::add_signal(signal);

	return signal;
}

void ScopeHardwareChannel::on_frame_begin(double timestamp, uint64_t samplerate)
{
	if (samplerate == 0) {
		qWarning() << "ScopeHardwareChannel::on_frame_begin(): samplerate is 0!";
		return;
	}

	qWarning() << "ScopeHardwareChannel::on_frame_begin(): samplerate = " << samplerate;
	next_signal_start_timestamp_ = timestamp;
	actual_samplerate_ = samplerate;

	if (actual_signal_)
		add_signal(actual_signal_->quantity(),
			actual_signal_->quantity_flags(), actual_signal_->unit());

	// Reset the actual signal, so a new one will be created for each frame.
	//actual_signal_ = nullptr;
}

} // namespace channels
} // namespace sv
