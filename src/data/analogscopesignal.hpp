/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_ANALOGSCOPESIGNAL_HPP
#define DATA_ANALOGSCOPESIGNAL_HPP

#include <deque>
#include <memory>
#include <set>

#include <QObject>

#include "src/data/analogbasesignal.hpp"
#include "src/data/datautil.hpp"

using std::deque;
using std::pair;
using std::set;
using std::shared_ptr;

namespace sv {
namespace data {

typedef pair<double, double> analog_scope_sample_t;

class AnalogSegment;

class AnalogScopeSignal : public AnalogBaseSignal
{
	Q_OBJECT

public:
	AnalogScopeSignal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		double signal_start_timestamp, uint64_t samplerate,
		const string &custom_name = "");

	/**
	 * Clear all samples from this signal.
	 */
	void clear() override;

	/**
	 * Return the sample at the given position.
	 */
	analog_scope_sample_t get_sample(size_t pos) const;

	/**
	 * Return the last captured sample.
	 */
	analog_scope_sample_t get_last_sample() const;

	/**
	 * Push multiple samples to the signal.
	 */
	void push_samples(void *data, uint64_t samples, double timestamp,
		uint64_t samplerate, size_t unit_size, int digits,
		int decimal_places) override;

	/**
	 * Return the last segment.
	 */
	shared_ptr<AnalogSegment> get_last_segment() const;

	/**
	 * Return a segment by its id.
	 */
	shared_ptr<AnalogSegment> get_segment(uint32_t segment_id) const;

	/**
	 * Complete/close the actual segment
	 */
	void complete_actual_segment();

	double signal_start_timestamp() const;
	double first_timestamp() const;
	double last_timestamp() const;

private:
	deque<shared_ptr<AnalogSegment>> segments_;
	shared_ptr<AnalogSegment> actual_segment_; // TODO: Is this even necessary?

	double signal_start_timestamp_;
	uint64_t actual_samplerate_; // TODO: Rename to cur_samplerate_; Don't set via ctor, but via methode/signal/slot/...?
								 // TODO: Is this even necessary?
	double time_stride_;
	double last_timestamp_;

Q_SIGNALS:
	void samples_added(uint32_t segment_id);
	void segment_added(uint32_t segment_id);

};

} // namespace data
} // namespace sv

#endif // DATA_ANALOGSCOPESIGNAL_HPP
