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

#include <limits>
#include <memory>
#include <utility>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include <QSignalSpy>
#include <vector>

#include "testutils/boostutil.hpp"
#include "testutils/signalutil.hpp"

BOOST_AUTO_TEST_SUITE(AnalogTimeSignalTest)


BOOST_AUTO_TEST_CASE(push_sample_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder()
		.start(1000.).push(1000., 1.).push(1001., 3.).push(1002., 2.).build();

	BOOST_CHECK_EQUAL(s->signal_start_timestamp(), 1000.);
	BOOST_CHECK_EQUAL(s->sample_count(), 3);

	BOOST_CHECK_EQUAL(s->get_sample(1, false), std::make_pair(1001., 3.));
	BOOST_CHECK_EQUAL(s->get_sample(1, true), std::make_pair(1., 3.));

	BOOST_CHECK_EQUAL(s->get_last_sample(false), std::make_pair(1002., 2.));
	BOOST_CHECK_EQUAL(s->get_last_sample(true), std::make_pair(2., 2.));

	BOOST_CHECK_EQUAL(s->first_timestamp(false), 1000.);
	BOOST_CHECK_EQUAL(s->first_timestamp(true), 0.);

	BOOST_CHECK_EQUAL(s->last_timestamp(false), 1002.);
	BOOST_CHECK_EQUAL(s->last_timestamp(true), 2.);

	BOOST_CHECK_EQUAL(s->min_value(), 1.);
	BOOST_CHECK_EQUAL(s->max_value(), 3.);
}

BOOST_AUTO_TEST_CASE(push_samples_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder().start(1000.).build();
	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);

	s->push_samples(std::vector<double>{2., 3., 10., 4.}, 1010., 4, 7, 3);

	BOOST_CHECK_EQUAL(s->signal_start_timestamp(), 1000.);
	BOOST_CHECK_EQUAL(s->sample_count(), 4);

	BOOST_CHECK_EQUAL(s->get_sample(1, false), std::make_pair(1010.25, 3.));
	BOOST_CHECK_EQUAL(s->get_sample(1, true), std::make_pair(10.25, 3.));

	BOOST_CHECK_EQUAL(s->get_last_sample(false), std::make_pair(1010.75, 4.));
	BOOST_CHECK_EQUAL(s->get_last_sample(true), std::make_pair(10.75, 4.));

	BOOST_CHECK_EQUAL(s->first_timestamp(false), 1010.);
	BOOST_CHECK_EQUAL(s->first_timestamp(true), 10.);

	BOOST_CHECK_EQUAL(s->last_timestamp(false), 1010.75);
	BOOST_CHECK_EQUAL(s->last_timestamp(true), 10.75);

	BOOST_CHECK_EQUAL(s->min_value(), 2.);
	BOOST_CHECK_EQUAL(s->max_value(), 10.);

	BOOST_CHECK_EQUAL(appended.count(), 1);
}

BOOST_AUTO_TEST_CASE(push_samples_empty_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder().start(1000.).build();
	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);

	s->push_samples(std::vector<double>{}, 1010., 4, 7, 3);

	BOOST_CHECK_EQUAL(s->signal_start_timestamp(), 1000.);
	BOOST_CHECK_EQUAL(s->sample_count(), 0);

	BOOST_CHECK_EQUAL(s->get_sample(1, false), std::make_pair(0., 0.));
	BOOST_CHECK_EQUAL(s->get_sample(1, true), std::make_pair(0., 0.));

	BOOST_CHECK_EQUAL(s->get_last_sample(false), std::make_pair(0., 0.));
	BOOST_CHECK_EQUAL(s->get_last_sample(true), std::make_pair(0., 0.));

	BOOST_CHECK_EQUAL(s->first_timestamp(false), 0.);
	BOOST_CHECK_EQUAL(s->first_timestamp(true), 0.);

	BOOST_CHECK_EQUAL(s->last_timestamp(false), 0.);
	BOOST_CHECK_EQUAL(s->last_timestamp(true), 0.);

	BOOST_CHECK_EQUAL(s->min_value(), std::numeric_limits<double>::max());
	BOOST_CHECK_EQUAL(s->max_value(), std::numeric_limits<double>::lowest());

	BOOST_CHECK_EQUAL(appended.count(), 0);
}

BOOST_AUTO_TEST_CASE(push_samples_samplerate_0_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder().start(1000.).build();
	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);

	s->push_samples(std::vector<double>{2., 3., 10., 4.}, 1010., 0, 7, 3);

	BOOST_CHECK_EQUAL(s->signal_start_timestamp(), 1000.);
	BOOST_CHECK_EQUAL(s->sample_count(), 4);

	BOOST_CHECK_EQUAL(s->get_sample(1, false), std::make_pair(1010., 3.));
	BOOST_CHECK_EQUAL(s->get_sample(1, true), std::make_pair(10., 3.));

	BOOST_CHECK_EQUAL(s->get_last_sample(false), std::make_pair(1010., 4.));
	BOOST_CHECK_EQUAL(s->get_last_sample(true), std::make_pair(10., 4.));

	BOOST_CHECK_EQUAL(s->first_timestamp(false), 1010.);
	BOOST_CHECK_EQUAL(s->first_timestamp(true), 10.);

	BOOST_CHECK_EQUAL(s->last_timestamp(false), 1010.);
	BOOST_CHECK_EQUAL(s->last_timestamp(true), 10.);

	BOOST_CHECK_EQUAL(s->min_value(), 2.);
	BOOST_CHECK_EQUAL(s->max_value(), 10.);

	BOOST_CHECK_EQUAL(appended.count(), 1);
}

BOOST_AUTO_TEST_CASE(get_value_at_timestamp_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder()
		.start(1000.).push(1000, 1.).push(1001, 3.).push(1002, 2.).build();

	double value;

	// Exact matches, relative
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(0., value, true), true);
	BOOST_CHECK_EQUAL(value, 1.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1., value, true), true);
	BOOST_CHECK_EQUAL(value, 3.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(2., value, true), true);
	BOOST_CHECK_EQUAL(value, 2.);

	// Exact matches, absolute
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1000., value, false), true);
	BOOST_CHECK_EQUAL(value, 1.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1001., value, false), true);
	BOOST_CHECK_EQUAL(value, 3.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1002., value, false), true);
	BOOST_CHECK_EQUAL(value, 2.);

	// Interpolate results, relative
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(0.5, value, true), true);
	BOOST_CHECK_EQUAL(value, 2.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1.5, value, true), true);
	BOOST_CHECK_EQUAL(value, 2.5);

	// Interpolate results, absolute
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1000.5, value, false), true);
	BOOST_CHECK_EQUAL(value, 2.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1001.5, value, false), true);
	BOOST_CHECK_EQUAL(value, 2.5);

	// Out of range
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(-.5, value, true), false);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(2.5, value, true), false);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(3., value, true), false);

	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(999., value, false), false);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(999.5, value, false), false);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1002.5, value, false), false);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1003., value, false), false);
}

BOOST_AUTO_TEST_CASE(clear_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder()
		.start(1000.).push(1000, 1.).push(1001, 3.).push(1002, 2.).build();
	QSignalSpy cleared(s.get(), &sv::data::AnalogTimeSignal::samples_cleared);

	s->clear();

	BOOST_CHECK_EQUAL(s->signal_start_timestamp(), 1000.);
	BOOST_CHECK_EQUAL(s->sample_count(), 0.);

	BOOST_CHECK_EQUAL(s->get_sample(1, false), std::make_pair(0., 0.));
	BOOST_CHECK_EQUAL(s->get_sample(1, true), std::make_pair(0., 0.));

	BOOST_CHECK_EQUAL(s->get_last_sample(false), std::make_pair(0., 0.));
	BOOST_CHECK_EQUAL(s->get_last_sample(true), std::make_pair(0., 0.));

	double value;
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1., value, true), false);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1., value, false), false);

	BOOST_CHECK_EQUAL(s->first_timestamp(true), 0.);
	BOOST_CHECK_EQUAL(s->first_timestamp(false), 0.);

	BOOST_CHECK_EQUAL(s->last_timestamp(true), 0.);
	BOOST_CHECK_EQUAL(s->last_timestamp(false), 0.);

	BOOST_CHECK_EQUAL(s->min_value(), std::numeric_limits<double>::max());
	BOOST_CHECK_EQUAL(s->max_value(), std::numeric_limits<double>::lowest());

	BOOST_CHECK_EQUAL(cleared.count(), 1);
}

BOOST_AUTO_TEST_CASE(signals_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder().build();

	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);
	QSignalSpy digits(s.get(), &sv::data::AnalogTimeSignal::digits_changed);

	s->push_sample(2., 1., 5, 2);
	s->push_sample(2., 2., 5, 2);

	BOOST_CHECK_EQUAL(appended.count(), 2);
	BOOST_CHECK_EQUAL(digits.count(), 1);
}

BOOST_AUTO_TEST_CASE(combine_signals_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder()
		.push(1., 1.).push(3., 2.).push(5., 3.).push(7., 4.).push(9., 5.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder()
		.push(6., 10.).push(8., 9.).push(10., 8.).push(12., 8.).build();
	size_t s1_size = 0;
	size_t s2_size = 0;

	auto combined_time = std::make_shared<std::vector<double>>();
	auto combined_s1 = std::make_shared<std::vector<double>>();
	auto combined_s2 = std::make_shared<std::vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(s1, s1_size, s2, s2_size,
		combined_time, combined_s1, combined_s2);

	std::vector<double> expected_time{6., 7., 8., 9.};
	std::vector<double> expected_s1{3.5, 4., 4.5, 5.};
	std::vector<double> expected_s2{10., 9.5, 9., 8.5};

	BOOST_CHECK_EQUAL(*combined_time, expected_time);
	BOOST_CHECK_EQUAL(*combined_s1, expected_s1);
	BOOST_CHECK_EQUAL(*combined_s2, expected_s2);
}

BOOST_AUTO_TEST_SUITE_END()
