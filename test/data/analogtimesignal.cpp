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

#include <cmath>
#include <memory>
#include <optional>
#include <utility>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include <QSignalSpy>
#include <vector>

#include "src/data/analogtimesignal.hpp"
#include "test/testutils/boostutil.hpp"
#include "test/testutils/signalutil.hpp"

using std::optional;

BOOST_AUTO_TEST_SUITE(AnalogTimeSignalTest)


BOOST_AUTO_TEST_CASE(empty_signal_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder().build();

	BOOST_CHECK_EQUAL(s->sample_count(), 0);

	BOOST_CHECK_EQUAL(s->get_sample(0, false), sv::data::analog_time_sample_t {});
	BOOST_CHECK_EQUAL(s->get_sample(0, true), sv::data::analog_time_sample_t {});

	BOOST_CHECK_EQUAL(s->get_last_sample(false), sv::data::analog_time_sample_t {});
	BOOST_CHECK_EQUAL(s->get_last_sample(true), sv::data::analog_time_sample_t {});

	BOOST_CHECK_EQUAL(s->first_timestamp(false), optional<double> {});
	BOOST_CHECK_EQUAL(s->first_timestamp(true), optional<double> {});

	BOOST_CHECK_EQUAL(s->last_timestamp(false), optional<double> {});
	BOOST_CHECK_EQUAL(s->last_timestamp(true), optional<double> {});

	BOOST_CHECK_EQUAL(s->min_value(), optional<double> {});
	BOOST_CHECK_EQUAL(s->max_value(), optional<double> {});
}

BOOST_AUTO_TEST_CASE(push_sample_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder()
		.push(1000., 1.).push(1001., 3.).push(1002., 2.).build();

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
	auto s = sv::test::testutils::TimeSignalBuilder().build();
	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);

	std::vector<double> samples{2., 3., 10., 4.};
	s->push_samples(
		samples.data(), samples.size(), 1010., 4, sizeof(double), 7, 3);

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
	auto s = sv::test::testutils::TimeSignalBuilder().build();
	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);

	std::vector<double> samples{};
	s->push_samples(
		samples.data(), samples.size(), 1010., 4, sizeof(double), 7, 3);

	BOOST_CHECK_EQUAL(s->sample_count(), 0);

	BOOST_CHECK_EQUAL(s->get_sample(1, false), sv::data::analog_time_sample_t {});
	BOOST_CHECK_EQUAL(s->get_sample(1, true), sv::data::analog_time_sample_t {});

	BOOST_CHECK_EQUAL(s->get_last_sample(false), sv::data::analog_time_sample_t {});
	BOOST_CHECK_EQUAL(s->get_last_sample(true), sv::data::analog_time_sample_t {});

	BOOST_CHECK_EQUAL(s->first_timestamp(false), optional<double> {});
	BOOST_CHECK_EQUAL(s->first_timestamp(true), optional<double> {});

	BOOST_CHECK_EQUAL(s->last_timestamp(false), optional<double> {});
	BOOST_CHECK_EQUAL(s->last_timestamp(true), optional<double> {});

	BOOST_CHECK_EQUAL(s->min_value(), optional<double> {});
	BOOST_CHECK_EQUAL(s->max_value(), optional<double> {});

	BOOST_CHECK_EQUAL(appended.count(), 0);
}

BOOST_AUTO_TEST_CASE(push_sample_NaN_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder().build();
	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);

	s->push_sample(std::numeric_limits<double>::quiet_NaN(), 1000., 7, 3);

	BOOST_CHECK_EQUAL(s->sample_count(), 1);
	BOOST_CHECK_EQUAL(appended.count(), 1);

	BOOST_CHECK(std::isnan(s->get_sample(0, false).value().second));
	BOOST_CHECK(std::isnan(s->get_last_sample(false).value().second));

	BOOST_CHECK_EQUAL(s->first_timestamp(false), 1000.);
	BOOST_CHECK_EQUAL(s->last_timestamp(false), 1000.);

	BOOST_CHECK(std::isnan(s->min_value().value()));
	BOOST_CHECK(std::isnan(s->max_value().value()));

	s->push_sample(5., 1001., 7, 3);

	BOOST_CHECK_EQUAL(s->sample_count(), 2);
	BOOST_CHECK_EQUAL(appended.count(), 2);

	BOOST_CHECK_EQUAL(s->get_sample(1, false), std::make_pair(1001., 5.));
	BOOST_CHECK_EQUAL(s->get_last_sample(false), std::make_pair(1001., 5.));

	BOOST_CHECK_EQUAL(s->first_timestamp(false), 1000.);
	BOOST_CHECK_EQUAL(s->last_timestamp(false), 1001.);

	BOOST_CHECK_EQUAL(s->min_value(), 5.);
	BOOST_CHECK_EQUAL(s->max_value(), 5.);
}

BOOST_AUTO_TEST_CASE(push_samples_samplerate_0_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder().build();
	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);

	std::vector<double> samples{2., 3., 10., 4.};
	s->push_samples(
		samples.data(), samples.size(), 1010., 0, sizeof(double), 7, 3);

	BOOST_CHECK_EQUAL(s->sample_count(), 4);

	BOOST_CHECK_EQUAL(s->get_sample(1, false), std::make_pair(1010., 3.));
	BOOST_CHECK_EQUAL(s->get_sample(1, true), std::make_pair(10., 3.));

	BOOST_CHECK_EQUAL(s->get_last_sample(false), std::make_pair(1010., 4.));
	BOOST_CHECK_EQUAL(s->get_last_sample(true), std::make_pair(10., 4.));

	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1., true), optional<double> {});
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1., false), optional<double> {});

	BOOST_CHECK_EQUAL(s->first_timestamp(false), 1010.);
	BOOST_CHECK_EQUAL(s->first_timestamp(true), 10.);

	BOOST_CHECK_EQUAL(s->last_timestamp(false), 1010.);
	BOOST_CHECK_EQUAL(s->last_timestamp(true), 10.);

	BOOST_CHECK_EQUAL(s->min_value(), 2.);
	BOOST_CHECK_EQUAL(s->max_value(), 10.);

	BOOST_CHECK_EQUAL(appended.count(), 1);
}

BOOST_AUTO_TEST_CASE(push_samples_samplerate_fp_error_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder().build();
	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);

	const size_t num_samples = 30000;
	const int samplerate = 3000;
	s->push_samples(std::vector<double>(num_samples, 1.), 0., samplerate, 7, 3);

	BOOST_CHECK_EQUAL(s->sample_count(), num_samples);

	BOOST_CHECK_EQUAL(s->first_timestamp(false), 0.);

	for (size_t pos=0; pos<num_samples; pos++) {
		double expected_time_abs = static_cast<double>(pos) / samplerate;
		BOOST_CHECK_SMALL(s->get_sample(pos, false)->first - expected_time_abs, 1e-12);
	}

	BOOST_CHECK_EQUAL(appended.count(), 1);
}

BOOST_AUTO_TEST_CASE(get_value_at_timestamp_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder()
		.push(1000, 1.).push(1001, 3.).push(1002, 2.).build();

	// Exact matches, relative
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(0., true), 1.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1., true), 3.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(2., true), 2.);

	// Exact matches, absolute
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1000., false), 1.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1001., false), 3.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1002., false), 2.);

	// Interpolate results, relative
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(0.5, true), 2.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1.5, true), 2.5);

	// Interpolate results, absolute
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1000.5, false), 2.);
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1001.5, false), 2.5);

	// Out of range
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(-.5, true), optional<double> {});
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(2.5, true), optional<double> {});
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(3., true), optional<double> {});

	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(999., false), optional<double> {});
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(999.5, false), optional<double> {});
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1002.5, false), optional<double> {});
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1003., false), optional<double> {});
}

BOOST_AUTO_TEST_CASE(clear_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder()
		.push(1000, 1.).push(1001, 3.).push(1002, 2.).build();
	QSignalSpy cleared(s.get(), &sv::data::AnalogTimeSignal::samples_cleared);

	s->clear();

	BOOST_CHECK_EQUAL(s->sample_count(), 0.);

	BOOST_CHECK_EQUAL(s->get_sample(1, false), sv::data::analog_time_sample_t {});
	BOOST_CHECK_EQUAL(s->get_sample(1, true), sv::data::analog_time_sample_t {});

	BOOST_CHECK_EQUAL(s->get_last_sample(false), sv::data::analog_time_sample_t {});
	BOOST_CHECK_EQUAL(s->get_last_sample(true), sv::data::analog_time_sample_t {});

	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1., true), optional<double> {});
	BOOST_CHECK_EQUAL(s->get_value_at_timestamp(1., false), optional<double> {});

	BOOST_CHECK_EQUAL(s->first_timestamp(false), optional<double> {});
	BOOST_CHECK_EQUAL(s->first_timestamp(true), optional<double> {});

	BOOST_CHECK_EQUAL(s->last_timestamp(false), optional<double> {});
	BOOST_CHECK_EQUAL(s->last_timestamp(true), optional<double> {});

	BOOST_CHECK_EQUAL(s->min_value(), optional<double> {});
	BOOST_CHECK_EQUAL(s->max_value(), optional<double> {});

	BOOST_CHECK_EQUAL(cleared.count(), 1);
}

BOOST_AUTO_TEST_CASE(push_after_clear_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder()
		.push(1000, 1.).push(1001, 3.).push(1002, 2.).build();
	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);

	s->clear();
	s->push_sample(4., 1005., 5, 2);

	BOOST_CHECK_EQUAL(s->sample_count(), 1.);

	BOOST_CHECK_EQUAL(s->get_sample(0, false),  std::make_pair(1005., 4.));
	BOOST_CHECK_EQUAL(s->get_sample(0, true),  std::make_pair(5., 4.));

	BOOST_CHECK_EQUAL(s->get_last_sample(false), std::make_pair(1005., 4.));
	BOOST_CHECK_EQUAL(s->get_last_sample(true), std::make_pair(5., 4.));

	BOOST_CHECK_EQUAL(s->first_timestamp(false), 1005.);
	BOOST_CHECK_EQUAL(s->first_timestamp(true), 5.);

	BOOST_CHECK_EQUAL(s->last_timestamp(false), 1005.);
	BOOST_CHECK_EQUAL(s->last_timestamp(true), 5.);

	BOOST_CHECK_EQUAL(s->min_value(), 4.);
	BOOST_CHECK_EQUAL(s->max_value(), 4.);

	BOOST_CHECK_EQUAL(appended.count(), 1);
}

BOOST_AUTO_TEST_CASE(qt_signals_test)
{
	auto s = sv::test::testutils::TimeSignalBuilder().build();

	QSignalSpy appended(s.get(), &sv::data::AnalogTimeSignal::sample_appended);
	QSignalSpy digits(s.get(), &sv::data::AnalogTimeSignal::digits_changed);

	s->push_sample(2., 1000., 5, 2);
	s->push_sample(2., 1001., 5, 2);

	BOOST_CHECK_EQUAL(appended.count(), 2);
	BOOST_CHECK_EQUAL(digits.count(), 1);
}

BOOST_AUTO_TEST_CASE(combine_signals_one_signal_empty_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder()
		.push(1001., 1.).push(1003., 2.).push(1005., 3.).push(1007., 4.)
		.push(1009., 5.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().build();
	size_t s1_size = 0;
	size_t s2_size = 0;

	auto combined_time = std::make_shared<std::vector<double>>();
	auto combined_s1 = std::make_shared<std::vector<double>>();
	auto combined_s2 = std::make_shared<std::vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(s1, s1_size, s2, s2_size,
		combined_time, combined_s1, combined_s2);

	std::vector<double> expected_time{};
	std::vector<double> expected_s1{};
	std::vector<double> expected_s2{};

	BOOST_CHECK_EQUAL(*combined_time, expected_time);
	BOOST_CHECK_EQUAL(*combined_s1, expected_s1);
	BOOST_CHECK_EQUAL(*combined_s2, expected_s2);
	BOOST_CHECK_EQUAL(s1_size, 0);
	BOOST_CHECK_EQUAL(s2_size, 0);
}

BOOST_AUTO_TEST_CASE(combine_signals_both_signals_empty_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder().build();
	auto s2 = sv::test::testutils::TimeSignalBuilder().build();
	size_t s1_size = 0;
	size_t s2_size = 0;

	auto combined_time = std::make_shared<std::vector<double>>();
	auto combined_s1 = std::make_shared<std::vector<double>>();
	auto combined_s2 = std::make_shared<std::vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(s1, s1_size, s2, s2_size,
		combined_time, combined_s1, combined_s2);

	std::vector<double> expected_time{};
	std::vector<double> expected_s1{};
	std::vector<double> expected_s2{};

	BOOST_CHECK_EQUAL(*combined_time, expected_time);
	BOOST_CHECK_EQUAL(*combined_s1, expected_s1);
	BOOST_CHECK_EQUAL(*combined_s2, expected_s2);
	BOOST_CHECK_EQUAL(s1_size, 0);
	BOOST_CHECK_EQUAL(s2_size, 0);
}

BOOST_AUTO_TEST_CASE(combine_signals_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder()
		.push(1001., 1.).push(1003., 2.).push(1005., 3.).push(1007., 4.)
		.push(1009., 5.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder()
		.push(1006., 10.).push(1008., 9.).push(1010., 8.).push(1012., 7.)
		.build();
	size_t s1_size = 0;
	size_t s2_size = 0;

	auto combined_time = std::make_shared<std::vector<double>>();
	auto combined_s1 = std::make_shared<std::vector<double>>();
	auto combined_s2 = std::make_shared<std::vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(s1, s1_size, s2, s2_size,
		combined_time, combined_s1, combined_s2);

	std::vector<double> expected_time{1006., 1007., 1008., 1009.};
	std::vector<double> expected_s1{3.5, 4., 4.5, 5.};
	std::vector<double> expected_s2{10., 9.5, 9., 8.5};

	BOOST_CHECK_EQUAL(*combined_time, expected_time);
	BOOST_CHECK_EQUAL(*combined_s1, expected_s1);
	BOOST_CHECK_EQUAL(*combined_s2, expected_s2);
	BOOST_CHECK_EQUAL(s1_size, 5);
	BOOST_CHECK_EQUAL(s2_size, 2);
}

BOOST_AUTO_TEST_CASE(combine_signals_s2_starts_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder()
		.push(1006., 10.).push(1008., 9.).push(1010., 8.).push(1012., 7.)
		.build();
	auto s2 = sv::test::testutils::TimeSignalBuilder()
		.push(1001., 1.).push(1003., 2.).push(1005., 3.).push(1007., 4.)
		.push(1009., 5.).build();
	size_t s1_size = 0;
	size_t s2_size = 0;

	auto combined_time = std::make_shared<std::vector<double>>();
	auto combined_s1 = std::make_shared<std::vector<double>>();
	auto combined_s2 = std::make_shared<std::vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(s1, s1_size, s2, s2_size,
		combined_time, combined_s1, combined_s2);

	std::vector<double> expected_time{1006., 1007., 1008., 1009.};
	std::vector<double> expected_s1{10., 9.5, 9., 8.5};
	std::vector<double> expected_s2{3.5, 4., 4.5, 5.};

	BOOST_CHECK_EQUAL(*combined_time, expected_time);
	BOOST_CHECK_EQUAL(*combined_s1, expected_s1);
	BOOST_CHECK_EQUAL(*combined_s2, expected_s2);
	BOOST_CHECK_EQUAL(s1_size, 2);
	BOOST_CHECK_EQUAL(s2_size, 5);
}

BOOST_AUTO_TEST_CASE(combine_signals_equal_timestamps_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder()
		.push(1001., 1.).push(1003., 2.).push(1005., 3.).push(1007., 4.)
		.push(1008., 5.).push(1009., 6.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder()
		.push(1006., 10.).push(1008., 9.).push(1010., 8.).push(1012., 7.)
		.build();
	size_t s1_size = 0;
	size_t s2_size = 0;

	auto combined_time = std::make_shared<std::vector<double>>();
	auto combined_s1 = std::make_shared<std::vector<double>>();
	auto combined_s2 = std::make_shared<std::vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(s1, s1_size, s2, s2_size,
		combined_time, combined_s1, combined_s2);

	std::vector<double> expected_time{1006., 1007., 1008., 1009.};
	std::vector<double> expected_s1{3.5, 4., 5., 6.};
	std::vector<double> expected_s2{10., 9.5, 9., 8.5};

	BOOST_CHECK_EQUAL(*combined_time, expected_time);
	BOOST_CHECK_EQUAL(*combined_s1, expected_s1);
	BOOST_CHECK_EQUAL(*combined_s2, expected_s2);
	BOOST_CHECK_EQUAL(s1_size, 6);
	BOOST_CHECK_EQUAL(s2_size, 2);
}

BOOST_AUTO_TEST_CASE(combine_signals_with_start_pos_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder()
		.push(1001., 1.).push(1003., 2.).push(1005., 3.).push(1007., 4.)
		.push(1009., 5.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder()
		.push(1006., 10.).push(1008., 9.).push(1010., 8.).push(1012., 7.)
		.build();
	size_t s1_size = 3;
	size_t s2_size = 1;

	auto combined_time = std::make_shared<std::vector<double>>();
	auto combined_s1 = std::make_shared<std::vector<double>>();
	auto combined_s2 = std::make_shared<std::vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(s1, s1_size, s2, s2_size,
		combined_time, combined_s1, combined_s2);

	std::vector<double> expected_time{1007., 1008., 1009.};
	std::vector<double> expected_s1{4., 4.5, 5.};
	std::vector<double> expected_s2{9.5, 9., 8.5};

	BOOST_CHECK_EQUAL(*combined_time, expected_time);
	BOOST_CHECK_EQUAL(*combined_s1, expected_s1);
	BOOST_CHECK_EQUAL(*combined_s2, expected_s2);
	BOOST_CHECK_EQUAL(s1_size, 5);
	BOOST_CHECK_EQUAL(s2_size, 2);
}

BOOST_AUTO_TEST_CASE(combine_signals_with_invalid_start_pos_1_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder()
		.push(1001., 1.).push(1003., 2.).push(1005., 3.).push(1007., 4.)
		.push(1009., 5.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder()
		.push(1006., 10.).push(1008., 9.).push(1010., 8.).push(1012., 7.)
		.build();
	size_t s1_size = 1;
	size_t s2_size = 1;

	auto combined_time = std::make_shared<std::vector<double>>();
	auto combined_s1 = std::make_shared<std::vector<double>>();
	auto combined_s2 = std::make_shared<std::vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(s1, s1_size, s2, s2_size,
		combined_time, combined_s1, combined_s2);

	std::vector<double> expected_time{};
	std::vector<double> expected_s1{};
	std::vector<double> expected_s2{};

	BOOST_CHECK_EQUAL(*combined_time, expected_time);
	BOOST_CHECK_EQUAL(*combined_s1, expected_s1);
	BOOST_CHECK_EQUAL(*combined_s2, expected_s2);
	BOOST_CHECK_EQUAL(s1_size, 1);
	BOOST_CHECK_EQUAL(s2_size, 1);
}

BOOST_AUTO_TEST_CASE(combine_signals_with_invalid_start_pos_2_test)
{
	auto s1 = sv::test::testutils::TimeSignalBuilder()
		.push(1001., 1.).push(1003., 2.).push(1005., 3.).push(1007., 4.)
		.push(1009., 5.).build();
	auto s2 = sv::test::testutils::TimeSignalBuilder()
		.push(1006., 10.).push(1008., 9.).push(1010., 8.).push(1012., 7.)
		.build();
	size_t s1_size = 10;
	size_t s2_size = 1;

	auto combined_time = std::make_shared<std::vector<double>>();
	auto combined_s1 = std::make_shared<std::vector<double>>();
	auto combined_s2 = std::make_shared<std::vector<double>>();

	sv::data::AnalogTimeSignal::combine_signals(s1, s1_size, s2, s2_size,
		combined_time, combined_s1, combined_s2);

	std::vector<double> expected_time{};
	std::vector<double> expected_s1{};
	std::vector<double> expected_s2{};

	BOOST_CHECK_EQUAL(*combined_time, expected_time);
	BOOST_CHECK_EQUAL(*combined_s1, expected_s1);
	BOOST_CHECK_EQUAL(*combined_s2, expected_s2);
	BOOST_CHECK_EQUAL(s1_size, 10);
	BOOST_CHECK_EQUAL(s2_size, 1);
}

BOOST_AUTO_TEST_SUITE_END()
