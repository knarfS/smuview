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

#include <QSignalSpy>

#include "src/data/analogsamplesignal.hpp"
#include "test/testutils/boostutil.hpp"
#include "test/testutils/signalutil.hpp"

using std::optional;

BOOST_AUTO_TEST_SUITE(AnalogSampleSignalTest)


BOOST_AUTO_TEST_CASE(empty_signal_test)
{
	auto s = sv::test::testutils::SampleSignalBuilder().build();

	BOOST_CHECK_EQUAL(s->sample_count(), 0);

	BOOST_CHECK_EQUAL(s->get_sample(0), sv::data::analog_pos_sample_t {});
	BOOST_CHECK_EQUAL(s->get_last_sample(), sv::data::analog_pos_sample_t {});

	BOOST_CHECK_EQUAL(s->first_pos(), optional<uint32_t> {});
	BOOST_CHECK_EQUAL(s->last_pos(), optional<uint32_t> {});

	BOOST_CHECK_EQUAL(s->min_value(), optional<double> {});
	BOOST_CHECK_EQUAL(s->max_value(), optional<double> {});
}

BOOST_AUTO_TEST_CASE(push_sample_test)
{
	auto s = sv::test::testutils::SampleSignalBuilder()
		.push(0, 1.).push(1, 3.).push(2, 2.).build();

	BOOST_CHECK_EQUAL(s->sample_count(), 3);

	BOOST_CHECK_EQUAL(s->get_sample(1), std::make_pair(1u, 3.));
	BOOST_CHECK_EQUAL(s->get_last_sample(), std::make_pair(2u, 2.));

	BOOST_CHECK_EQUAL(s->first_pos(), 0);
	BOOST_CHECK_EQUAL(s->last_pos(), 2);

	BOOST_CHECK_EQUAL(s->min_value(), 1.);
	BOOST_CHECK_EQUAL(s->max_value(), 3.);
}

BOOST_AUTO_TEST_CASE(push_sample_start_at_5_test)
{
	auto s = sv::test::testutils::SampleSignalBuilder()
		.push(5, 1.).push(6, 3.).push(7, 2.).build();

	BOOST_CHECK_EQUAL(s->sample_count(), 3);

	BOOST_CHECK_EQUAL(s->get_sample(1), std::make_pair(6u, 3.));
	BOOST_CHECK_EQUAL(s->get_last_sample(), std::make_pair(7u, 2.));

	BOOST_CHECK_EQUAL(s->first_pos(), 5);
	BOOST_CHECK_EQUAL(s->last_pos(), 7);

	BOOST_CHECK_EQUAL(s->min_value(), 1.);
	BOOST_CHECK_EQUAL(s->max_value(), 3.);
}

BOOST_AUTO_TEST_CASE(push_sample_NaN_test)
{
	auto s = sv::test::testutils::SampleSignalBuilder().build();
	QSignalSpy appended(s.get(), &sv::data::AnalogSampleSignal::sample_appended);

	s->push_sample(std::numeric_limits<double>::quiet_NaN(), 0, 7, 3);

	BOOST_CHECK_EQUAL(s->sample_count(), 1);
	BOOST_CHECK_EQUAL(appended.count(), 1);

	BOOST_CHECK(std::isnan(s->get_sample(0).value().second));
	BOOST_CHECK(std::isnan(s->get_last_sample().value().second));

	BOOST_CHECK_EQUAL(s->first_pos(), 0);
	BOOST_CHECK_EQUAL(s->last_pos(), 0);

	BOOST_CHECK(std::isnan(s->min_value().value()));
	BOOST_CHECK(std::isnan(s->max_value().value()));

	s->push_sample(5., 1, 7, 3);

	BOOST_CHECK_EQUAL(s->sample_count(), 2);
	BOOST_CHECK_EQUAL(appended.count(), 2);

	BOOST_CHECK_EQUAL(s->get_sample(1), std::make_pair(1u, 5.));
	//BOOST_CHECK_EQUAL(s->get_last_sample(), std::make_pair(1u, 5.));

	BOOST_CHECK_EQUAL(s->first_pos(), 0);
	BOOST_CHECK_EQUAL(s->last_pos(), 1);

	BOOST_CHECK_EQUAL(s->min_value(), 5.);
	BOOST_CHECK_EQUAL(s->max_value(), 5.);
}

BOOST_AUTO_TEST_CASE(clear_test)
{
	auto s = sv::test::testutils::SampleSignalBuilder()
		.push(0, 1.).push(1, 3.).push(2, 2.).build();
	QSignalSpy cleared(s.get(), &sv::data::AnalogSampleSignal::samples_cleared);

	s->clear();

	BOOST_CHECK_EQUAL(s->sample_count(), 0.);

	BOOST_CHECK_EQUAL(s->get_sample(1), sv::data::analog_pos_sample_t {});
	BOOST_CHECK_EQUAL(s->get_last_sample(), sv::data::analog_pos_sample_t {});

	BOOST_CHECK_EQUAL(s->first_pos(), optional<double> {});
	BOOST_CHECK_EQUAL(s->last_pos(), optional<double> {});

	BOOST_CHECK_EQUAL(s->min_value(), optional<double> {});
	BOOST_CHECK_EQUAL(s->max_value(), optional<double> {});

	BOOST_CHECK_EQUAL(cleared.count(), 1);
}

BOOST_AUTO_TEST_CASE(push_after_clear_test)
{
	auto s = sv::test::testutils::SampleSignalBuilder()
		.push(0, 1.).push(1, 3.).push(2, 2.).build();
	QSignalSpy appended(s.get(), &sv::data::AnalogSampleSignal::sample_appended);

	s->clear();
	s->push_sample(4., 5, 5, 2);

	BOOST_CHECK_EQUAL(s->sample_count(), 1.);

	BOOST_CHECK_EQUAL(s->get_sample(0),  std::make_pair(5u, 4.));
	BOOST_CHECK_EQUAL(s->get_last_sample(), std::make_pair(5u, 4.));

	BOOST_CHECK_EQUAL(s->first_pos(), 5);
	BOOST_CHECK_EQUAL(s->last_pos(), 5);

	BOOST_CHECK_EQUAL(s->min_value(), 4.);
	BOOST_CHECK_EQUAL(s->max_value(), 4.);

	BOOST_CHECK_EQUAL(appended.count(), 1);
}

BOOST_AUTO_TEST_CASE(qt_signals_test)
{
	auto s = sv::test::testutils::SampleSignalBuilder().build();

	QSignalSpy appended(s.get(), &sv::data::AnalogSampleSignal::sample_appended);
	QSignalSpy digits(s.get(), &sv::data::AnalogSampleSignal::digits_changed);

	s->push_sample(2., 0, 5, 2);
	s->push_sample(2., 1, 5, 2);

	BOOST_CHECK_EQUAL(appended.count(), 2);
	BOOST_CHECK_EQUAL(digits.count(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
