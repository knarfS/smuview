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

#ifndef DATA_ANALOGSIGNAL_HPP
#define DATA_ANALOGSIGNAL_HPP

#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <QObject>

#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"

using std::pair;
using std::set;
using std::shared_ptr;
using std::vector;

namespace sv {
namespace data {

typedef pair<double, double> sample_t;

class AnalogSignal : public BaseSignal
{
	Q_OBJECT

public:
	AnalogSignal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		double signal_start_timestamp);

	void clear();

	size_t get_sample_count() const;
	vector<double> get_samples(size_t start_sample, size_t end_sample) const;
	sample_t get_sample(size_t pos, bool relative_time) const;

	void push_sample(void *sample, double timestamp,
		size_t unit_size, int digits, int decimal_places);

	int digits() const;
	int decimal_places() const;
	double signal_start_timestamp() const;
	double first_timestamp(bool relative_time) const;
	double last_timestamp(bool relative_time) const;
	double last_value() const;
	double min_value() const;
	double max_value() const;

private:
	shared_ptr<vector<double>> time_;
	shared_ptr<vector<double>> data_;
	size_t sample_count_;
	int digits_;
	int decimal_places_;
	double signal_start_timestamp_;
	double last_timestamp_;
	double last_value_;
	double min_value_;
	double max_value_;

	static const size_t size_of_float_ = sizeof(float);
	static const size_t size_of_double_ = sizeof(double);

public Q_SLOTS:
	void on_channel_start_timestamp_changed(double);

Q_SIGNALS:
	void signal_start_timestamp_changed(double);
	void samples_cleared();
	void sample_added();
	void digits_changed(const int, const int);

};

} // namespace data
} // namespace sv

#endif // DATA_ANALOGSIGNAL_HPP
