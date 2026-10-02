/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <map>
#include <memory>
#include <vector>
#include <boost/test/unit_test.hpp>

#include "src/utils/exportutil.hpp"
#include "test/testutils/boostutil.hpp"
#include "test/testutils/signalutil.hpp"

using std::map;
using std::shared_ptr;
using std::vector;

BOOST_AUTO_TEST_SUITE(ExportUtilTest)

BOOST_AUTO_TEST_CASE(align_timestamps_absolute_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().start(10.)
		.push(10., 1.).push(11., 2.).push(12., 3.).push(13., 4.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().start(5.)
		.push(8., 10.).push(9., 20.).push(10., 30.).push(11., 40.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.);

	BOOST_CHECK_EQUAL(result.columnNames.size(), 3);
	BOOST_CHECK_EQUAL(result.columnNames, (vector<string> {"Time", "test", "test"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 6);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 8.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (map<size_t, double> {{1, 10.}}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 9.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (map<size_t, double> {{1, 20.}}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 10.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (map<size_t, double> {{1, 30.}, {0, 1.}}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 11.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (map<size_t, double> {{0, 2.}, {1, 40.}}));
	BOOST_CHECK_EQUAL(result.rows.at(4).time, 12.);
	BOOST_CHECK_EQUAL(result.rows.at(4).data, (map<size_t, double> {{0, 3.}}));
	BOOST_CHECK_EQUAL(result.rows.at(5).time, 13.);
	BOOST_CHECK_EQUAL(result.rows.at(5).data, (map<size_t, double> {{0, 4.}}));
}

BOOST_AUTO_TEST_CASE(align_timestamps_relative_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().start(5.)
		.push(10., 1.).push(11., 2.).push(12., 3.).push(13., 4.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().start(5.)
		.push(8., 10.).push(9., 20.).push(10., 30.).push(11., 40.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, true, 0.);

	BOOST_CHECK_EQUAL(result.columnNames.size(), 3);
	BOOST_CHECK_EQUAL(result.columnNames, (vector<string> {"Time", "test", "test"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 6);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 3.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (map<size_t, double> {{1, 10.}}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 4.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (map<size_t, double> {{1, 20.}}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 5.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (map<size_t, double> {{1, 30.}, {0, 1.}}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 6.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (map<size_t, double> {{0, 2.}, {1, 40.}}));
	BOOST_CHECK_EQUAL(result.rows.at(4).time, 7.);
	BOOST_CHECK_EQUAL(result.rows.at(4).data, (map<size_t, double> {{0, 3.}}));
	BOOST_CHECK_EQUAL(result.rows.at(5).time, 8.);
	BOOST_CHECK_EQUAL(result.rows.at(5).data, (map<size_t, double> {{0, 4.}}));
}

BOOST_AUTO_TEST_CASE(align_timestamps_tolerance_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().start(10.)
		.push(10.5, 1.).push(11.5, 2.).push(12.5, 3.).push(13.5, 4.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().start(5.)
		.push(8., 10.).push(9., 20.).push(10., 30.).push(11., 40.).build();
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals{s1, s2};

	auto result = sv::utils::exportutil::align_timestamps(signals, false, 0.5);

	BOOST_CHECK_EQUAL(result.columnNames.size(), 3);
	BOOST_CHECK_EQUAL(result.columnNames, (vector<string> {"Time", "test", "test"}));
	BOOST_CHECK_EQUAL(result.rows.size(), 6);
	BOOST_CHECK_EQUAL(result.rows.at(0).time, 8.);
	BOOST_CHECK_EQUAL(result.rows.at(0).data, (map<size_t, double> {{1, 10.}}));
	BOOST_CHECK_EQUAL(result.rows.at(1).time, 9.);
	BOOST_CHECK_EQUAL(result.rows.at(1).data, (map<size_t, double> {{1, 20.}}));
	BOOST_CHECK_EQUAL(result.rows.at(2).time, 10.);
	BOOST_CHECK_EQUAL(result.rows.at(2).data, (map<size_t, double> {{1, 30.}, {0, 1.}}));
	BOOST_CHECK_EQUAL(result.rows.at(3).time, 11.);
	BOOST_CHECK_EQUAL(result.rows.at(3).data, (map<size_t, double> {{0, 2.}, {1, 40.}}));
	BOOST_CHECK_EQUAL(result.rows.at(4).time, 12.5);
	BOOST_CHECK_EQUAL(result.rows.at(4).data, (map<size_t, double> {{0, 3.}}));
	BOOST_CHECK_EQUAL(result.rows.at(5).time, 13.5);
	BOOST_CHECK_EQUAL(result.rows.at(5).data, (map<size_t, double> {{0, 4.}}));
}

BOOST_AUTO_TEST_SUITE_END()
