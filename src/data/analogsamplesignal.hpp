/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2026 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_ANALOGSAMPLESIGNAL_HPP
#define DATA_ANALOGSAMPLESIGNAL_HPP

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

typedef optional<pair<uint32_t, double>> analog_pos_sample_t;

class AnalogSampleSignal : public AnalogBaseSignal
{
	Q_OBJECT

public:
	AnalogSampleSignal(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		const string &custom_name = "");

	/**
	 * Clear all samples from this signal.
	 */
	void clear() override;

	/**
	 * Return the sample at the given position.
	 */
	analog_pos_sample_t get_sample(uint32_t pos) const;

	/**
	 * Return the last captured sample.
	 */
	analog_pos_sample_t get_last_sample() const;

	/**
	 * Push a single sample to the signal.
	 */
	void push_sample(double sample, uint32_t pos,
		int total_digits, int sr_digits);

	optional<uint32_t> first_pos() const;
	optional<uint32_t> last_pos() const;

private:
	vector<uint32_t> pos_;

};

} // namespace data
} // namespace sv

#endif // DATA_ANALOGSAMPLESIGNAL_HPP
