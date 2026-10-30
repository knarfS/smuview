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
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "src/data/analogsamplesignal.hpp"
#include "src/data/analogtimesignal.hpp"

namespace sv::test::testutils {

class TimeSignalBuilder
{

public:
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
		auto sig = std::make_shared<data::AnalogTimeSignal>(
			data::Quantity::Voltage,
			std::set<data::QuantityFlag>{data::QuantityFlag::DC},
			data::Unit::Volt,
			nullptr, name_);
		for (auto [t, v] : samples_) {
			sig->push_sample(v, t, 7, 3);
		}
		return sig;
	}

private:
	std::string name_ = "test";
	std::vector<std::pair<double, double>> samples_;
};

class SampleSignalBuilder
{

public:
	SampleSignalBuilder &name(std::string n)
	{
		name_ = std::move(n);
		return *this;
	}

	SampleSignalBuilder &push(uint32_t p, double v)
	{
		samples_.emplace_back(p, v);
		return *this;
	}

	std::shared_ptr<data::AnalogSampleSignal> build() const
	{
		auto sig = std::make_shared<data::AnalogSampleSignal>(
			data::Quantity::Voltage,
			std::set<data::QuantityFlag>{data::QuantityFlag::DC},
			data::Unit::Volt,
			nullptr, name_);
		for (auto [p, v] : samples_) {
			sig->push_sample(v, p, 7, 3);
		}
		return sig;
	}

private:
	std::string name_ = "test";
	std::vector<std::pair<uint32_t, double>> samples_;
};

} // namespace sv::test::testutils

#endif // SMUVIEW_TEST_TESTUTIL_SIGNALUTILS_HPP
