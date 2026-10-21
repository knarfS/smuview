/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef SMUVIEW_TEST_TESTUTIL_SIGNALUTILS_HPP
#define SMUVIEW_TEST_TESTUTIL_SIGNALUTILS_HPP

#include <memory>
#include <string>
#include <vector>

#include "src/data/analogtimesignal.hpp"

namespace sv::test::testutils {

class TimeSignalBuilder
{

public:
	TimeSignalBuilder &start(double ts)
	{
		start_ = ts;
		return *this;
	}

	TimeSignalBuilder &name(std::string n)
	{
		name_ = std::move(n);
		return *this;
	}

	TimeSignalBuilder &push(double t, double v)
	{
		samples_.emplace_back(t, v);
		return *this;
	}

	std::shared_ptr<data::AnalogTimeSignal> build() const
	{
		// Adjust enum names to what datautil.hpp actually has
		auto sig = std::make_shared<data::AnalogTimeSignal>(
			data::Quantity::Voltage,
			std::set<data::QuantityFlag>{data::QuantityFlag::DC},
			data::Unit::Volt,
			nullptr, start_, name_);
		for (auto [t, v] : samples_) {
			double x = v;
			sig->push_sample(&x, t, sizeof(double), 7, 3);
		}
		return sig;
	}

private:
	double start_ = 0.;
	std::string name_ = "test";
	std::vector<std::pair<double, double>> samples_;
};

} // namespace sv::test::testutils

#endif // SMUVIEW_TEST_TESTUTIL_SIGNALUTILS_HPP
