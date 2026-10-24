/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_ANALOGTIMESIGNAL_HPP
#define DATA_ANALOGTIMESIGNAL_HPP

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <QObject>

#include "src/data/analogbasesignal.hpp"
#include "src/data/datautil.hpp"

using std::optional;
using std::pair;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {
namespace data {

typedef optional<pair<double, double>> analog_time_sample_t;

struct AnalogTimeSignalBoundaries {
	double first_timestamp;
	double last_timestamp;
	double min_value;
	double max_value;
};

class AnalogTimeSignal : public AnalogBaseSignal
{
	Q_OBJECT

public:
	AnalogTimeSignal(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		double signal_start_timestamp,
		const string &custom_name = "");

	/**
	 * Clear all samples from this signal.
	 */
	void clear() override;

	/**
	 * Return the sample at the given position.
	 */
	analog_time_sample_t get_sample(size_t pos, bool relative_time) const;

	/**
	 * Return the last captured sample.
	 */
	analog_time_sample_t get_last_sample(bool relative_time) const;

	/**
	 * Return the value at the given timestamp. If there is no exactty matching
	 * timestamp, the value is linearly interpolated. No value can be found or
	 * interpolated, if the timestamp is smaller than the first timestamp in the
	 * signal or bigger than the last timestamp in the signal.
	 *
	 * @param timestamp The timestamp for the value to return
	 * @param relative_time Use time relative to the session start time
	 *
	 * @return The found/interpolated value at the given timestamp
	 */
	optional<double> get_value_at_timestamp(
		double timestamp, bool relative_time) const;

	/**
	 * Push a single sample to the signal.
	 */
	void push_sample(double sample, double timestamp,
		int total_digits, int sr_digits);

	/**
	 * Push multiple samples to the signal.
	 */
	void push_samples(const vector<double> &samples, double timestamp,
		uint64_t samplerate, int total_digits, int sr_digits);

	double signal_start_timestamp() const;
	optional<double> first_timestamp(bool relative_time) const;
	optional<double> last_timestamp(bool relative_time) const;

	optional<AnalogTimeSignalBoundaries> get_boundaries(
		bool relative_time) const;

	/**
	 * Combine two signals with each other.
	 *
	 * E.g.:
	 * | Time | S1 | S2 | combined Time | combined S1 | combined S2 |
	 * |------|----|----|---------------|-------------|-------------|
	 * |    1 |  1 |    |               |             |             |
	 * |    3 |  2 |    |               |             |             |
	 * |    5 |  3 |    |               |             |             |
	 * |    6 |    | 10 |             6 |         3.5 |          10 |
	 * |    7 |  4 |    |             7 |           4 |         9.5 |
	 * |    8 |    |  9 |             8 |         4.5 |           9 |
	 * |    9 |  5 |    |             9 |           5 |         8.5 |
	 * |   10 |    |  8 |               |             |             |
	 * |   12 |    |  7 |               |             |             |
	 *
	 * TODO: Use std::deque<double>& instead of shared_ptr<vector<double>>?
	 */
	static void combine_signals(
		shared_ptr<AnalogTimeSignal> signal1, size_t &signal1_pos,
		shared_ptr<AnalogTimeSignal> signal2, size_t &signal2_pos,
		shared_ptr<vector<double>> time_vector,
		shared_ptr<vector<double>> data1_vector,
		shared_ptr<vector<double>> data2_vector);

private:
	vector<double> time_;
	double signal_start_timestamp_;

public Q_SLOTS:
	void on_channel_start_timestamp_changed(double timestamp);

Q_SIGNALS:
	void signal_start_timestamp_changed(double timestamp);

};

} // namespace data
} // namespace sv

#endif // DATA_ANALOGTIMESIGNAL_HPP
