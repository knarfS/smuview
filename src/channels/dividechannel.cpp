/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2022 Frank Stettner <frank-stettner@gmx.net>
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

#include "dividechannel.hpp"
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

DivideChannel::DivideChannel(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		shared_ptr<data::AnalogTimeSignal> dividend_signal,
		shared_ptr<data::AnalogTimeSignal> divisor_signal,
		shared_ptr<devices::BaseDevice> parent_device,
		const set<string> &channel_group_names,
		const string &channel_name,
		double channel_start_timestamp) :
	MathChannel(quantity, quantity_flags, unit,
		parent_device, channel_group_names, channel_name,
		channel_start_timestamp),
	dividend_signal_(dividend_signal),
	divisor_signal_(divisor_signal),
	dividend_signal_pos_(0),
	divisor_signal_pos_(0)
{
	assert(dividend_signal_);
	assert(divisor_signal_);

	if (dividend_signal->total_digits() >= divisor_signal->total_digits())
		total_digits_ = dividend_signal->total_digits();
	else
		total_digits_ = divisor_signal->total_digits();

	// Use the lower sr_digits value to get a greater resoulution
	if (dividend_signal->sr_digits() < divisor_signal->sr_digits())
		sr_digits_ = dividend_signal->sr_digits();
	else
		sr_digits_ = divisor_signal->sr_digits();

	connect(dividend_signal_.get(), &data::AnalogTimeSignal::sample_appended,
		this, &DivideChannel::on_sample_appended);
	connect(divisor_signal_.get(), &data::AnalogTimeSignal::sample_appended,
		this, &DivideChannel::on_sample_appended);
}

void DivideChannel::on_sample_appended()
{
	lock_guard<mutex> lock(sample_append_mutex_);

	shared_ptr<vector<double>> time = make_shared<vector<double>>();
	shared_ptr<vector<double>> dividend_data = make_shared<vector<double>>();
	shared_ptr<vector<double>> divisor_data = make_shared<vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(
		dividend_signal_, dividend_signal_pos_,
		divisor_signal_, divisor_signal_pos_,
		time, dividend_data, divisor_data);

	for (size_t i=0; i<time->size(); i++) {
		// Division
		double value;
		if (divisor_data->at(i) == 0) {
			if (dividend_data->at(i) > 0)
				value = std::numeric_limits<double>::max();
			else
				value = std::numeric_limits<double>::lowest();
		}
		else {
			value = dividend_data->at(i) / divisor_data->at(i);
		}
		push_sample(value, time->at(i));
	}
}

} // namespace channels
} // namespace sv
