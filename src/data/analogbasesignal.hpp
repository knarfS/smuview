/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2022 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_ANALOGBASESIGNAL_HPP
#define DATA_ANALOGBASESIGNAL_HPP

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <QObject>

#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"

using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {
namespace data {

class AnalogBaseSignal : public BaseSignal
{
	Q_OBJECT

public:
	AnalogBaseSignal(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel,
		const string &custom_name);

	/**
	 * Return the number of samples in this signal.
	 */
	size_t sample_count() const override;

	/**
	 * Number of total digits (count) of the measured value.
	 * NOTE: Not implemented in sigrok yet. There is no good way to get the
	 *       total number of digits for the analog payload. Therefore
	 *       `total_digits` is initialized with some reasonable value.
	 */
	int total_digits() const;
	/** digits from ....digits */
	int sr_digits() const;
	double last_value() const;
	double min_value() const;
	double max_value() const;

protected:
	vector<double> data_;
	size_t sample_count_;
	int total_digits_;
	int sr_digits_;
	double last_value_;
	double min_value_;
	double max_value_;

Q_SIGNALS:
	void samples_cleared();
	void sample_appended();
	void digits_changed(const int total_digits, const int sr_digits);

};

} // namespace data
} // namespace sv

#endif // DATA_ANALOGBASESIGNAL_HPP
