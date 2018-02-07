/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef CHANNELS_MULTIPLYSFCHANNEL_HPP
#define CHANNELS_MULTIPLYSFCHANNEL_HPP

#include <memory>
#include <set>

#include <QObject>

#include "src/channels/basechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/data/datautil.hpp"

using std::set;
using std::shared_ptr;

namespace sv {

namespace data {
class AnalogSignal;
}

namespace devices {
class Device;
}

namespace channels {

class MultiplySFChannel : public MathChannel
{
	Q_OBJECT

public:
	MultiplySFChannel(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<data::AnalogSignal> signal,
		double factor,
		shared_ptr<devices::Device> parent_device,
		const QString channel_group_name,
		QString channel_name,
		double channel_start_timestamp);

private:
	shared_ptr<data::AnalogSignal> signal_;
	double factor_;
	size_t next_signal_pos_;

private Q_SLOTS:
	void on_sample_added();

};

} // namespace channels
} // namespace sv

#endif // CHANNELS_MULTIPLYSFCHANNEL_HPP
