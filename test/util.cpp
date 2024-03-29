/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2015 Jens Steinhauser <jens.steinhauser@gmail.com>
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
#include <math.h>
#include <boost/test/unit_test.hpp>

#include "src/util.hpp"
#include "test/test.hpp"

using namespace sv::util;
using ts = sv::util::Timestamp;

using std::bind;

namespace {
	QChar mu = QChar(0x03BC);

	sv::util::SIPrefix unspecified = sv::util::SIPrefix::unspecified;
	sv::util::SIPrefix yocto       = sv::util::SIPrefix::yocto;
	sv::util::SIPrefix nano        = sv::util::SIPrefix::nano;
/*	sv::util::SIPrefix micro       = sv::util::SIPrefix::micro; // Not currently used */
	sv::util::SIPrefix milli       = sv::util::SIPrefix::milli;
	sv::util::SIPrefix none        = sv::util::SIPrefix::none;
	sv::util::SIPrefix kilo        = sv::util::SIPrefix::kilo;
	sv::util::SIPrefix yotta       = sv::util::SIPrefix::yotta;

/*	sv::util::TimeUnit Time = sv::util::TimeUnit::Time; // Not currently used */
}  // namespace

BOOST_AUTO_TEST_SUITE(UtilTest)

BOOST_AUTO_TEST_CASE(exponent_test)
{
	BOOST_CHECK_EQUAL(exponent(SIPrefix::yocto), -24);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::zepto), -21);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::atto),  -18);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::femto), -15);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::pico),  -12);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::nano),   -9);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::micro),  -6);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::milli),  -3);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::none),    0);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::kilo),    3);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::mega),    6);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::giga),    9);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::tera),   12);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::peta),   15);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::exa),    18);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::zetta),  21);
	BOOST_CHECK_EQUAL(exponent(SIPrefix::yotta),  24);
}

BOOST_AUTO_TEST_CASE(format_value_si_test)
{
	QString value_str("");
	QString si_prefix_str("");

	/* Common values for DMMs */

	format_value_si(4635000000.           , -1, -6, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "G");

	si_prefix_str = "";
	format_value_si( 463500000.           , -1, -5, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.5");
	BOOST_CHECK_EQUAL(si_prefix_str, "M");

	si_prefix_str = "";
	format_value_si(  46350000.           , -1, -4, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.35");
	BOOST_CHECK_EQUAL(si_prefix_str, "M");

	si_prefix_str = "";
	format_value_si(   4635000.           , -1, -3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "M");

	si_prefix_str = "";
	format_value_si(    463500.           , -1, -2, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.5");
	BOOST_CHECK_EQUAL(si_prefix_str, "k");

	si_prefix_str = "";
	format_value_si(     46350.           , -1, -1, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.35");
	BOOST_CHECK_EQUAL(si_prefix_str, "k");

	si_prefix_str = "";
	format_value_si(      4635.           , -1,  0, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "k");

	si_prefix_str = "";
	format_value_si(       463.5           , -1,  1, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.5");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(        46.35          , -1,  2, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.35");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(         4.635         , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(          .4635        , -1,  4, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.5");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(          .04635       , -1,  5, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.35");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(          .004635      , -1,  6, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(          .0004635     , -1,  7, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.5");
	BOOST_CHECK_EQUAL(si_prefix_str, mu);

	si_prefix_str = "";
	format_value_si(          .00004635    , -1,  8, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.35");
	BOOST_CHECK_EQUAL(si_prefix_str, mu);

	si_prefix_str = "";
	format_value_si(          .000004635   , -1,  9, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, mu);

	si_prefix_str = "";
	format_value_si(          .0000004635  , -1, 10, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.5");
	BOOST_CHECK_EQUAL(si_prefix_str, "n");

	si_prefix_str = "";
	format_value_si(          .00000004635 , -1, 11, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.35");
	BOOST_CHECK_EQUAL(si_prefix_str, "n");

	si_prefix_str = "";
	format_value_si(          .000000004635, -1, 12, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "n");

	/* Common values for power supplies */

	si_prefix_str = "";
	format_value_si(       123.456         , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.456");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(       123.4567        , -1,  4, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.4567");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	/* Edge cases */

	si_prefix_str = "";
	format_value_si(1234e25, -1,  -25, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "12340");
	BOOST_CHECK_EQUAL(si_prefix_str, "Y");

	si_prefix_str = "";
	format_value_si(1234e23, -1, -23, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.4");
	BOOST_CHECK_EQUAL(si_prefix_str, "Y");

	si_prefix_str = "";
	format_value_si(.4635e-23, -1, 27, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "y");

	si_prefix_str = "";
	format_value_si(.4635e-26, -1, 30, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.004635");
	BOOST_CHECK_EQUAL(si_prefix_str, "y");

	si_prefix_str = "";
	format_value_si(0, -1,  4, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.0000");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(0, -1,  1, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.0");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	// si_prefix_str = "";
	// format_value_si(NAN, -1,  4, value_str, si_prefix_str, false);
	// BOOST_CHECK_EQUAL(value_str, "nan");
	// BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(std::numeric_limits<double>::infinity(), -1,  4, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "inf");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(std::numeric_limits<double>::max(), -1,  4, value_str, si_prefix_str, false);
	//BOOST_CHECK_EQUAL(value_str, "0.0000");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(std::numeric_limits<double>::lowest(), -1,  4, value_str, si_prefix_str, false);
	//BOOST_CHECK_EQUAL(value_str, "0.0000");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	/* Some more values from the libsigrok tests */

	si_prefix_str = "";
	format_value_si(        12.0           , -1,  1, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "12.0");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(        12.0           , -1, -1, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.01");
	BOOST_CHECK_EQUAL(si_prefix_str, "k");

	si_prefix_str = "";
	format_value_si(      1024.0           , -1,  0, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "1.024");
	BOOST_CHECK_EQUAL(si_prefix_str, "k");

	si_prefix_str = "";
	format_value_si(      1024.0           , -1, -1, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "1.02");
	BOOST_CHECK_EQUAL(si_prefix_str, "k");

	si_prefix_str = "";
	format_value_si(      1024.0           , -1, -3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "1");
	BOOST_CHECK_EQUAL(si_prefix_str, "k");

	si_prefix_str = "";
	format_value_si(        12.0e5         , -1, 0, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "1.200000");
	BOOST_CHECK_EQUAL(si_prefix_str, "M");

	si_prefix_str = "";
	format_value_si(         0.123456      , -1, 0, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(         0.123456      , -1, 1, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.1");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(         0.123456      , -1, 2, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.12");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si(         0.123456      , -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(         0.123456      , -1, 4, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.4");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(         0.123456      , -1, 5, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.45");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(         0.123456      , -1, 6, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.456");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(         0.123456      , -1, 7, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.4560");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(         0.0123        , -1, 4, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "12.3");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(         0.00123       , -1, 5, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "1.23");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(         0.000123      , -1, 4, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.1");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(         0.000123      , -1, 5, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.12");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si(         0.000123      , -1, 6, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123");
	BOOST_CHECK_EQUAL(si_prefix_str, mu);

	si_prefix_str = "";
	format_value_si(         0.000123      , -1, 7, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.0");
	BOOST_CHECK_EQUAL(si_prefix_str, mu);

	si_prefix_str = "";
	format_value_si(         0.0001      , -1, 4, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.1");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");
}

BOOST_AUTO_TEST_CASE(format_value_si_autoscale_test)
{
	QString value_str("");
	QString si_prefix_str("");

	/* Common values for DMMs */

	format_value_si_autoscale(4635000000.           , -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "G");

	si_prefix_str = "";
	format_value_si_autoscale( 463500000.           , -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.500");
	BOOST_CHECK_EQUAL(si_prefix_str, "M");

	si_prefix_str = "";
	format_value_si_autoscale(  46350000.           , -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.350");
	BOOST_CHECK_EQUAL(si_prefix_str, "M");

	si_prefix_str = "";
	format_value_si_autoscale(   4635000.           , -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "M");

	si_prefix_str = "";
	format_value_si_autoscale(    463500.           , -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.500");
	BOOST_CHECK_EQUAL(si_prefix_str, "k");

	si_prefix_str = "";
	format_value_si_autoscale(     46350.           , -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.350");
	BOOST_CHECK_EQUAL(si_prefix_str, "k");

	si_prefix_str = "";
	format_value_si_autoscale(      4635.           , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "k");

	si_prefix_str = "";
	format_value_si_autoscale(       463.5           , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.500");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si_autoscale(        46.35          , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.350");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si_autoscale(         4.635         , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si_autoscale(          .4635        , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.500");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si_autoscale(          .04635       , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.350");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si_autoscale(          .004635      , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si_autoscale(          .0004635     , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.500");
	BOOST_CHECK_EQUAL(si_prefix_str, mu);

	si_prefix_str = "";
	format_value_si_autoscale(          .00004635    , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.350");
	BOOST_CHECK_EQUAL(si_prefix_str, mu);

	si_prefix_str = "";
	format_value_si_autoscale(          .000004635   , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, mu);

	si_prefix_str = "";
	format_value_si_autoscale(          .0000004635  , -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "463.500");
	BOOST_CHECK_EQUAL(si_prefix_str, "n");

	si_prefix_str = "";
	format_value_si_autoscale(          .00000004635 , -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "46.350");
	BOOST_CHECK_EQUAL(si_prefix_str, "n");

	si_prefix_str = "";
	format_value_si_autoscale(          .000000004635, -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "n");

	/* Common values for power supplies */

	si_prefix_str = "";
	format_value_si_autoscale(       123.456         , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.456");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si_autoscale(       123.4567        , -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.456");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	/* Edge cases */

	si_prefix_str = "";
	format_value_si_autoscale(1234e25, -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "12340.000");
	BOOST_CHECK_EQUAL(si_prefix_str, "Y");

	si_prefix_str = "";
	format_value_si_autoscale(1234e23, -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.400");
	BOOST_CHECK_EQUAL(si_prefix_str, "Y");

	si_prefix_str = "";
	format_value_si_autoscale(.4635e-23, -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "4.635");
	BOOST_CHECK_EQUAL(si_prefix_str, "y");

	si_prefix_str = "";
	format_value_si_autoscale(.4635e-26, -1, 6, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.004635");
	BOOST_CHECK_EQUAL(si_prefix_str, "y");

	si_prefix_str = "";
	format_value_si_autoscale(0, -1,  2, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.00");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si_autoscale(0, -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "0.000");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	// si_prefix_str = "";
	// format_value_si(NAN, -1,  3, value_str, si_prefix_str, false);
	// BOOST_CHECK_EQUAL(value_str, "nan");
	// BOOST_CHECK_EQUAL(si_prefix_str, "");

	si_prefix_str = "";
	format_value_si_autoscale(std::numeric_limits<double>::infinity(), -1,  3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "inf");
	BOOST_CHECK_EQUAL(si_prefix_str, "");

	/* Some more values from the libsigrok tests */

	// si_prefix_str = "";
	// format_value_si(        12.0           , -1,  1, value_str, si_prefix_str, false);
	// BOOST_CHECK_EQUAL(value_str, "12.0");
	// BOOST_CHECK_EQUAL(si_prefix_str, "");
	//
	// si_prefix_str = "";
	// format_value_si(        12.0           , -1, -1, value_str, si_prefix_str, false);
	// BOOST_CHECK_EQUAL(value_str, "0.01");
	// BOOST_CHECK_EQUAL(si_prefix_str, "k");
	//
	// si_prefix_str = "";
	// format_value_si(      1024.0           , -1,  0, value_str, si_prefix_str, false);
	// BOOST_CHECK_EQUAL(value_str, "1.024");
	// BOOST_CHECK_EQUAL(si_prefix_str, "k");
	//
	// si_prefix_str = "";
	// format_value_si(      1024.0           , -1, -1, value_str, si_prefix_str, false);
	// BOOST_CHECK_EQUAL(value_str, "1.02");
	// BOOST_CHECK_EQUAL(si_prefix_str, "k");
	//
	// si_prefix_str = "";
	// format_value_si(      1024.0           , -1, -3, value_str, si_prefix_str, false);
	// BOOST_CHECK_EQUAL(value_str, "1");
	// BOOST_CHECK_EQUAL(si_prefix_str, "k");
	//
	// si_prefix_str = "";
	// format_value_si(        12.0e5         , -1, 0, value_str, si_prefix_str, false);
	// BOOST_CHECK_EQUAL(value_str, "1.200000");
	// BOOST_CHECK_EQUAL(si_prefix_str, "M");

	si_prefix_str = "";
	format_value_si_autoscale(         0.123456      , -1, 0, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si_autoscale(         0.123456      , -1, 1, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.4");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si_autoscale(         0.123456      , -1, 2, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.45");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si_autoscale(         0.123456      , -1, 3, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.456");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si_autoscale(         0.123456      , -1, 4, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.4560");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si_autoscale(         0.0123        , -1, 2, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "12.30");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si_autoscale(         0.00123       , -1, 2, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "1.23");
	BOOST_CHECK_EQUAL(si_prefix_str, "m");

	si_prefix_str = "";
	format_value_si_autoscale(         0.000123      , -1, 2, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "123.00");
	BOOST_CHECK_EQUAL(si_prefix_str, mu);

	si_prefix_str = "";
	format_value_si_autoscale(         0.0001      , -1, 2, value_str, si_prefix_str, false);
	BOOST_CHECK_EQUAL(value_str, "100.00");
	BOOST_CHECK_EQUAL(si_prefix_str, mu);
}

BOOST_AUTO_TEST_CASE(format_time_si_test)
{
	// check prefix calculation

	BOOST_CHECK_EQUAL(format_time_si(ts("0")), "0 s");

	BOOST_CHECK_EQUAL(format_time_si(ts("1e-24")),    "+1 ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-23")),   "+10 ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-22")),  "+100 ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-21")),    "+1 zs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-20")),   "+10 zs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-19")),  "+100 zs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-18")),    "+1 as");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-17")),   "+10 as");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-16")),  "+100 as");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-15")),    "+1 fs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-14")),   "+10 fs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-13")),  "+100 fs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-12")),    "+1 ps");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-11")),   "+10 ps");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-10")),  "+100 ps");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-9")),     "+1 ns");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-8")),    "+10 ns");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-7")),   "+100 ns");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-6")),    QString("+1 ") + mu + "s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-5")),   QString("+10 ") + mu + "s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-4")),  QString("+100 ") + mu + "s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-3")),     "+1 ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-2")),    "+10 ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-1")),   "+100 ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e0")),       "+1 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e1")),      "+10 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e2")),     "+100 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e3")),      "+1 ks");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e4")),     "+10 ks");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e5")),    "+100 ks");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e6")),      "+1 Ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e7")),     "+10 Ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e8")),    "+100 Ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e9")),      "+1 Gs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e10")),    "+10 Gs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e11")),   "+100 Gs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e12")),     "+1 Ts");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e13")),    "+10 Ts");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e14")),   "+100 Ts");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e15")),     "+1 Ps");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e16")),    "+10 Ps");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e17")),   "+100 Ps");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e18")),     "+1 Es");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e19")),    "+10 Es");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e20")),   "+100 Es");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e21")),     "+1 Zs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e22")),    "+10 Zs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e23")),   "+100 Zs");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e24")),     "+1 Ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e25")),    "+10 Ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e26")),   "+100 Ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e27")),  "+1000 Ys");

	BOOST_CHECK_EQUAL(format_time_si(ts("1234")),              "+1 ks");
	BOOST_CHECK_EQUAL(format_time_si(ts("1234"), kilo, 3), "+1.234 ks");
	BOOST_CHECK_EQUAL(format_time_si(ts("1234.5678")),         "+1 ks");

	// check prefix

	BOOST_CHECK_EQUAL(format_time_si(ts("1e-24"), yocto),    "+1 ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-21"), yocto), "+1000 ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("0"), yocto),         "0 ys");

	BOOST_CHECK_EQUAL(format_time_si(ts("1e-4"), milli),         "+0 ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-4"), milli, 1),     "+0.1 ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("1000"), milli),    "+1000000 ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("0"), milli),              "0 ms");

	BOOST_CHECK_EQUAL(format_time_si(ts("1e-1"), none),       "+0 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-1"), none, 1),  "+0.1 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e-1"), none, 2), "+0.10 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1"), none),          "+1 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e1"), none),       "+10 s");

	BOOST_CHECK_EQUAL(format_time_si(ts("1e23"), yotta),       "+0 Ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e23"), yotta, 1),  "+0.1 Ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("1e27"), yotta),    "+1000 Ys");
	BOOST_CHECK_EQUAL(format_time_si(ts("0"), yotta),           "0 Ys");

	// check precision, rounding

	BOOST_CHECK_EQUAL(format_time_si(ts("1.2345678")),                         "+1 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1.4")),                               "+1 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1.5")),                               "+2 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1.9")),                               "+2 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1.2345678"), unspecified, 2),      "+1.23 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1.2345678"), unspecified, 3),     "+1.235 s");
	BOOST_CHECK_EQUAL(format_time_si(ts("1.2345678"), milli, 3),       "+1234.568 ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("1.2345678"), milli, 0),           "+1235 ms");
	BOOST_CHECK_EQUAL(format_time_si(ts("1.2"), unspecified, 3),           "+1.200 s");

	// check unit and sign

	BOOST_CHECK_EQUAL(format_time_si(ts("-1"), none, 0, "V", true),  "-1 V");
	BOOST_CHECK_EQUAL(format_time_si(ts("-1"), none, 0, "V", false), "-1 V");
	BOOST_CHECK_EQUAL(format_time_si(ts("1"), none, 0, "V", true),   "+1 V");
	BOOST_CHECK_EQUAL(format_time_si(ts("1"), none, 0, "V", false),   "1 V");
}

BOOST_AUTO_TEST_CASE(format_time_si_adjusted_test)
{
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts("-1.5"), milli), "-1500 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts("-1.0"), milli), "-1000 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts("-0.2"), milli),  "-200 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts("-0.1"), milli),  "-100 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "0.0"), milli),     "0 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "0.1"), milli),  "+100 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "0.2"), milli),  "+200 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "0.3"), milli),  "+300 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "0.4"), milli),  "+400 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "0.5"), milli),  "+500 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "0.6"), milli),  "+600 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "0.7"), milli),  "+700 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "0.8"), milli),  "+800 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "0.9"), milli),  "+900 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.0"), milli), "+1000 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.1"), milli), "+1100 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.2"), milli), "+1200 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.3"), milli), "+1300 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.4"), milli), "+1400 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.5"), milli), "+1500 ms");

	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.5"), milli, 6), "+1500.000 ms");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.5"), nano,  6), "+1500000000 ns");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.5"), nano,  8), "+1500000000 ns");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.5"), nano,  9), "+1500000000 ns");
	BOOST_CHECK_EQUAL(format_time_si_adjusted(ts( "1.5"), nano, 10), "+1500000000.0 ns");
}

BOOST_AUTO_TEST_CASE(format_time_minutes_test)
{
	using namespace std::placeholders;

	auto fmt = bind(format_time_minutes, _1, _2, true);

	BOOST_CHECK_EQUAL(fmt(ts(    0), 0),    "+0:00");
	BOOST_CHECK_EQUAL(fmt(ts(    1), 0),    "+0:01");
	BOOST_CHECK_EQUAL(fmt(ts(   59), 0),    "+0:59");
	BOOST_CHECK_EQUAL(fmt(ts(   60), 0),    "+1:00");
	BOOST_CHECK_EQUAL(fmt(ts(   -1), 0),    "-0:01");
	BOOST_CHECK_EQUAL(fmt(ts(  -59), 0),    "-0:59");
	BOOST_CHECK_EQUAL(fmt(ts(  -60), 0),    "-1:00");
	BOOST_CHECK_EQUAL(fmt(ts(  100), 0),    "+1:40");
	BOOST_CHECK_EQUAL(fmt(ts( -100), 0),    "-1:40");
	BOOST_CHECK_EQUAL(fmt(ts( 4000), 0), "+1:06:40");
	BOOST_CHECK_EQUAL(fmt(ts(-4000), 0), "-1:06:40");
	BOOST_CHECK_EQUAL(fmt(ts(12000), 0), "+3:20:00");
	BOOST_CHECK_EQUAL(fmt(ts(15000), 0), "+4:10:00");
	BOOST_CHECK_EQUAL(fmt(ts(20000), 0), "+5:33:20");
	BOOST_CHECK_EQUAL(fmt(ts(25000), 0), "+6:56:40");

	BOOST_CHECK_EQUAL(fmt(ts("10641906.007008009"), 0), "+123:04:05:06");
	BOOST_CHECK_EQUAL(fmt(ts("10641906.007008009"), 1), "+123:04:05:06.0");
	BOOST_CHECK_EQUAL(fmt(ts("10641906.007008009"), 2), "+123:04:05:06.01");
	BOOST_CHECK_EQUAL(fmt(ts("10641906.007008009"), 3), "+123:04:05:06.007");
	BOOST_CHECK_EQUAL(fmt(ts("10641906.007008009"), 4), "+123:04:05:06.007 0");
	BOOST_CHECK_EQUAL(fmt(ts("10641906.007008009"), 5), "+123:04:05:06.007 01");
	BOOST_CHECK_EQUAL(fmt(ts("10641906.007008009"), 6), "+123:04:05:06.007 008");
	BOOST_CHECK_EQUAL(fmt(ts("10641906.007008009"), 7), "+123:04:05:06.007 008 0");
	BOOST_CHECK_EQUAL(fmt(ts("10641906.007008009"), 8), "+123:04:05:06.007 008 01");
	BOOST_CHECK_EQUAL(fmt(ts("10641906.007008009"), 9), "+123:04:05:06.007 008 009");

	BOOST_CHECK_EQUAL(format_time_minutes(ts(   0), 0, false),  "0:00");
	BOOST_CHECK_EQUAL(format_time_minutes(ts( 100), 0, false),  "1:40");
	BOOST_CHECK_EQUAL(format_time_minutes(ts(-100), 0, false), "-1:40");
}

BOOST_AUTO_TEST_CASE(count_int_digits_test)
{
	BOOST_CHECK_EQUAL(count_int_digits( 101), 3);
	BOOST_CHECK_EQUAL(count_int_digits( 100), 3);
	BOOST_CHECK_EQUAL(count_int_digits(  99), 2);
	BOOST_CHECK_EQUAL(count_int_digits(  11), 2);
	BOOST_CHECK_EQUAL(count_int_digits(  10), 2);
	BOOST_CHECK_EQUAL(count_int_digits(   9), 1);
	BOOST_CHECK_EQUAL(count_int_digits(   1), 1);
	BOOST_CHECK_EQUAL(count_int_digits(   0), 0);
	BOOST_CHECK_EQUAL(count_int_digits(  -1), 1);
	BOOST_CHECK_EQUAL(count_int_digits(  -9), 1);
	BOOST_CHECK_EQUAL(count_int_digits( -10), 2);
	BOOST_CHECK_EQUAL(count_int_digits( -11), 2);
	BOOST_CHECK_EQUAL(count_int_digits( -99), 2);
	BOOST_CHECK_EQUAL(count_int_digits(-100), 3);
	BOOST_CHECK_EQUAL(count_int_digits(-101), 3);
}

BOOST_AUTO_TEST_CASE(count_double_digits_test)
{
	BOOST_CHECK_EQUAL(count_double_digits( 100.   , 1. ),  3);
	BOOST_CHECK_EQUAL(count_double_digits(  10.   , 1. ),  2);
	BOOST_CHECK_EQUAL(count_double_digits(   1.   , 1. ),  1);
	BOOST_CHECK_EQUAL(count_double_digits(   0.   , 1.  ), 0);
	BOOST_CHECK_EQUAL(count_double_digits( 100.   ,  .1),  4);
	BOOST_CHECK_EQUAL(count_double_digits(  10.   ,  .1),  3);
	BOOST_CHECK_EQUAL(count_double_digits(   1.   ,  .1),  2);
	BOOST_CHECK_EQUAL(count_double_digits(   0.   ,  .1 ), 1);
	BOOST_CHECK_EQUAL(count_double_digits( 100.   ,  .01), 5);
	BOOST_CHECK_EQUAL(count_double_digits(  10.   ,  .01), 4);
	BOOST_CHECK_EQUAL(count_double_digits(   1.   ,  .01), 3);
	BOOST_CHECK_EQUAL(count_double_digits(   0.   ,  .01), 2);

	BOOST_CHECK_EQUAL(count_double_digits(   1.1  ,  .01), 3);
	BOOST_CHECK_EQUAL(count_double_digits(   1.01 ,  .01), 3);
	BOOST_CHECK_EQUAL(count_double_digits(   1.001,  .01), 4);
}

BOOST_AUTO_TEST_CASE(count_decimal_places_test)
{
	BOOST_CHECK_EQUAL(count_decimal_places(   .00001), 5);
	BOOST_CHECK_EQUAL(count_decimal_places(   .0001),  4);
	BOOST_CHECK_EQUAL(count_decimal_places(   .001),   3);
	BOOST_CHECK_EQUAL(count_decimal_places(   .01),    2);
	BOOST_CHECK_EQUAL(count_decimal_places(   .1),     1);
	BOOST_CHECK_EQUAL(count_decimal_places(  0.),      0);
	BOOST_CHECK_EQUAL(count_decimal_places(  1.),      0);
	BOOST_CHECK_EQUAL(count_decimal_places( 10.),      0);
	BOOST_CHECK_EQUAL(count_decimal_places(100.),      0);

	BOOST_CHECK_EQUAL(count_decimal_places(   .00002), 5);
	BOOST_CHECK_EQUAL(count_decimal_places(   .00003), 5);
	BOOST_CHECK_EQUAL(count_decimal_places(   .00004), 5);
	BOOST_CHECK_EQUAL(count_decimal_places(   .00005), 5);
	BOOST_CHECK_EQUAL(count_decimal_places(   .00006), 5);
	BOOST_CHECK_EQUAL(count_decimal_places(   .00007), 5);
	BOOST_CHECK_EQUAL(count_decimal_places(   .00008), 5);
	BOOST_CHECK_EQUAL(count_decimal_places(   .00009), 5);

	BOOST_CHECK_EQUAL(count_decimal_places(   .11   ), 2);
	BOOST_CHECK_EQUAL(count_decimal_places(   .111  ), 3);
	BOOST_CHECK_EQUAL(count_decimal_places(   .1111 ), 4);
	BOOST_CHECK_EQUAL(count_decimal_places(   .9999 ), 4);

	BOOST_CHECK_EQUAL(count_decimal_places(  1.001  ), 3);
	BOOST_CHECK_EQUAL(count_decimal_places(  1.00001), 5);
	BOOST_CHECK_EQUAL(count_decimal_places(   .00007), 5);
}

BOOST_AUTO_TEST_CASE(get_sr_digits_test)
{
	BOOST_CHECK_EQUAL(get_sr_digits(1000000.       ), -6);
	BOOST_CHECK_EQUAL(get_sr_digits( 100000.       ), -5);
	BOOST_CHECK_EQUAL(get_sr_digits(  10000.       ), -4);
	BOOST_CHECK_EQUAL(get_sr_digits(   1000.       ), -3);
	BOOST_CHECK_EQUAL(get_sr_digits(    100.       ), -2);
	BOOST_CHECK_EQUAL(get_sr_digits(     10.       ), -1);
	BOOST_CHECK_EQUAL(get_sr_digits(      1.       ),  0);
	BOOST_CHECK_EQUAL(get_sr_digits(      0.       ),  0);
	BOOST_CHECK_EQUAL(get_sr_digits(       .1      ),  1);
	BOOST_CHECK_EQUAL(get_sr_digits(       .01     ),  2);
	BOOST_CHECK_EQUAL(get_sr_digits(       .001    ),  3);
	BOOST_CHECK_EQUAL(get_sr_digits(       .0001   ),  4);
	BOOST_CHECK_EQUAL(get_sr_digits(       .00001  ),  5);
	BOOST_CHECK_EQUAL(get_sr_digits(       .000001 ),  6);
	BOOST_CHECK_EQUAL(get_sr_digits(       .0000001),  7);

	BOOST_CHECK_EQUAL(get_sr_digits(1100000.       ), -5);
	BOOST_CHECK_EQUAL(get_sr_digits(1110000.       ), -4);
	BOOST_CHECK_EQUAL(get_sr_digits(1111000.       ), -3);
	BOOST_CHECK_EQUAL(get_sr_digits(1111100.       ), -2);
	BOOST_CHECK_EQUAL(get_sr_digits(1111110.       ), -1);
	BOOST_CHECK_EQUAL(get_sr_digits(1111111.       ),  0);
	BOOST_CHECK_EQUAL(get_sr_digits(1111111.1      ),  1);
	BOOST_CHECK_EQUAL(get_sr_digits(1111111.11     ),  2);
	BOOST_CHECK_EQUAL(get_sr_digits(1111111.111    ),  3);

	BOOST_CHECK_EQUAL(get_sr_digits(       .02     ),  2);
	BOOST_CHECK_EQUAL(get_sr_digits(       .03     ),  2);
	BOOST_CHECK_EQUAL(get_sr_digits(       .04     ),  2);
	BOOST_CHECK_EQUAL(get_sr_digits(       .05     ),  2);
	BOOST_CHECK_EQUAL(get_sr_digits(       .06     ),  2);
	BOOST_CHECK_EQUAL(get_sr_digits(       .07     ),  2);
	BOOST_CHECK_EQUAL(get_sr_digits(       .08     ),  2);
	BOOST_CHECK_EQUAL(get_sr_digits(       .09     ),  2);
}

BOOST_AUTO_TEST_SUITE_END()
