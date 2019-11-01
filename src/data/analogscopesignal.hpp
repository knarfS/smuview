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

#ifndef DATA_ANALOGSCOPESIGNAL_HPP
#define DATA_ANALOGSCOPESIGNAL_HPP

#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <QObject>

#include "src/data/analogbasesignal.hpp"
#include "src/data/datautil.hpp"

using std::pair;
using std::set;
using std::shared_ptr;
using std::vector;

namespace sv {
namespace data {

typedef pair<double, double> analog_time_sample_t;

class AnalogScopeSignal : public AnalogBaseSignal
{
	Q_OBJECT

public:
	AnalogScopeSignal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		double signal_start_timestamp, uint64_t samplerate);

	/**
	 * Clear all samples from this signal.
	 */
	void clear() override;

	/**
	 * Return the sample at the given position.
	 */
	analog_time_sample_t get_sample(size_t pos) const;

	/**
	 * Return the last captured sample.
	 */
	analog_time_sample_t get_last_sample() const;

	/**
	 * Push multiple samples to the signal.
	 */
	void push_samples(void *data, uint64_t samples, double timestamp,
		size_t unit_size, int digits, int decimal_places) override;

	double signal_start_timestamp() const;
	double first_timestamp() const;
	double last_timestamp() const;

private:
	double signal_start_timestamp_;
	double time_stride_;
	double last_timestamp_;

};

} // namespace data
} // namespace sv

#endif // DATA_ANALOGSCOPESIGNAL_HPP
