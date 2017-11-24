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

#ifndef DATA_ANALOG_HPP
#define DATA_ANALOG_HPP

#include <memory>

#include <QObject>

#include "basedata.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {
namespace data {

class AnalogSegment;

class Analog : public BaseData
{
	Q_OBJECT

public:
	Analog();

	void clear();

	size_t get_sample_count() const;
	vector<double> get_samples(size_t start_sample, size_t end_sample) const;
	double get_sample(size_t pos) const;

	void push_sample(void *sample);

	double last_value() const;
	double min_value() const;
	double max_value() const;

Q_SIGNALS:
	void samples_cleared();

private:
	shared_ptr<vector<double>> data_;
	size_t sample_count_;

	double last_value_;
	double min_value_;
	double max_value_;
};

} // namespace data
} // namespace sv

#endif // DATA_ANALOG_HPP
