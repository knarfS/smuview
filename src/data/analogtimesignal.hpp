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

#ifndef DATA_ANALOGTIMESIGNAL_HPP
#define DATA_ANALOGTIMESIGNAL_HPP

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

class AnalogTimeSignal : public AnalogBaseSignal
{
	Q_OBJECT

public:
	AnalogTimeSignal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		double signal_start_timestamp);

	/**
	 * Clear all samples from this signal.
	 */
	void clear() override;

	/**
	 * Return the sample at the given position.
	 */
	analog_time_sample_t get_sample(size_t pos, bool relative_time) const;

	/**
	 * Return the value at the given timestamp in &value. If there is no
	 * exactty matching timestamp, the value is linearly interpolated. No
	 * value can be found/interpolated, if the timestamp is smaller than the
	 * first timestamp in the signal or bigger than the last timestamp in the
	 * signal.
	 *
	 * @param timestamp The timestamp for the value to return.
	 * @param value The found/interpolated value at the given timestamp.
	 * @param relative_time Use time relative to the session start time.
	 *
	 * @return true if a value was found/interpolated, false if not.
	 */
	bool get_value_at_timestamp(
		double timestamp, double &value, bool relative_time) const;

	/**
	 * Push a single sample to the signal.
	 *
	 * TODO: Can this be removed?
	 */
	void push_sample(void *sample, double timestamp,
		size_t unit_size, int digits, int decimal_places);

	/**
	 * Push multiple samples to the signal.
	 */
	void push_samples(void *data, uint64_t samples, double timestamp,
		uint64_t samplerate, size_t unit_size, int digits, int decimal_places);

	double signal_start_timestamp() const;
	double first_timestamp(bool relative_time) const;
	double last_timestamp(bool relative_time) const;

	static void combine_signals(
		shared_ptr<AnalogTimeSignal> signal1, size_t &signal1_pos,
		shared_ptr<AnalogTimeSignal> signal2, size_t &signal2_pos,
		shared_ptr<vector<double>> time_vector,
		shared_ptr<vector<double>> data1_vector,
		shared_ptr<vector<double>> data2_vector);

private:
	shared_ptr<vector<double>> time_;
	double signal_start_timestamp_;
	double last_timestamp_;

public Q_SLOTS:
	void on_channel_start_timestamp_changed(double);

Q_SIGNALS:
	void signal_start_timestamp_changed(double);

};

} // namespace data
} // namespace sv

#endif // DATA_ANALOGTIMESIGNAL_HPP
