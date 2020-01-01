/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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
#include <mutex>
#include <set>
#include <string>

#include <QDebug>

#include "multiplysschannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::lock_guard;
using std::make_shared;
using std::mutex;
using std::set;
using std::string;

namespace sv {
namespace channels {

MultiplySSChannel::MultiplySSChannel(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<data::AnalogTimeSignal> signal1,
		shared_ptr<data::AnalogTimeSignal> signal2,
		shared_ptr<devices::BaseDevice> parent_device,
		set<string> channel_group_names,
		string channel_name,
		double channel_start_timestamp) :
	MathChannel(quantity, quantity_flags, unit,
		parent_device, channel_group_names, channel_name,
		channel_start_timestamp),
	signal1_(signal1),
	signal2_(signal2),
	signal1_pos_(0),
	signal2_pos_(0)
{
	assert(signal1_);
	assert(signal2_);

	if (signal1_->digits() >= signal2_->digits())
		digits_ = signal1_->digits();
	else
		digits_ = signal2_->digits();

	if (signal1_->decimal_places() >= signal2_->decimal_places())
		decimal_places_ = signal1_->decimal_places();
	else
		decimal_places_ = signal2_->decimal_places();

	connect(signal1_.get(), SIGNAL(sample_appended()),
		this, SLOT(on_sample_appended()));
	connect(signal2_.get(), SIGNAL(sample_appended()),
		this, SLOT(on_sample_appended()));
}

void MultiplySSChannel::on_sample_appended()
{
	lock_guard<mutex> lock(sample_append_mutex_);

	shared_ptr<vector<double>> time = make_shared<vector<double>>();
	shared_ptr<vector<double>> signal1_data = make_shared<vector<double>>();
	shared_ptr<vector<double>> signal2_data = make_shared<vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(
		signal1_, signal1_pos_,
		signal2_, signal2_pos_,
		time, signal1_data, signal2_data);

	for (size_t i=0; i<time->size(); i++) {
		push_sample(signal1_data->at(i) * signal2_data->at(i), time->at(i));
	}
}

} // namespace devices
} // namespace sv
