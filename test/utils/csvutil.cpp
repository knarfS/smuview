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

#include <string>
#include <vector>
#include <boost/test/unit_test.hpp>

#include "src/utils/csvutil.hpp"
#include "test/testutils/boostutil.hpp"

using std::string;
using std::vector;

BOOST_AUTO_TEST_SUITE(CSVUtilTest)

BOOST_AUTO_TEST_CASE(format_csv_value)
{
	BOOST_CHECK_EQUAL(sv::utils::csvutil::format_csv_value("test", ';'), "test");
	BOOST_CHECK_EQUAL(sv::utils::csvutil::format_csv_value("tes;t", ','), "tes;t");
	BOOST_CHECK_EQUAL(sv::utils::csvutil::format_csv_value("tes;t", ';'), "\"tes;t\"");
	BOOST_CHECK_EQUAL(sv::utils::csvutil::format_csv_value("tes\nt", ';'), "\"tes\nt\"");
	BOOST_CHECK_EQUAL(sv::utils::csvutil::format_csv_value("tes\rt", ';'), "\"tes\rt\"");
	BOOST_CHECK_EQUAL(sv::utils::csvutil::format_csv_value("tes\"t", ';'), "\"tes\"\"t\"");
	BOOST_CHECK_EQUAL(sv::utils::csvutil::format_csv_value("tes\"t\"", ';'), "\"tes\"\"t\"\"\"");
	BOOST_CHECK_EQUAL(sv::utils::csvutil::format_csv_value("t\n\re;s\"t\"", ';'), "\"t\n\re;s\"\"t\"\"\"");
}

BOOST_AUTO_TEST_CASE(parse_csv_line) {
	BOOST_CHECK_EQUAL(sv::utils::csvutil::parse_csv_line("1", ';'),
		(vector<string> {"1"}));
	BOOST_CHECK_EQUAL(sv::utils::csvutil::parse_csv_line("1;;", ';'),
		(vector<string> {"1", "", ""}));
	BOOST_CHECK_EQUAL(sv::utils::csvutil::parse_csv_line("1;;3", ';'),
		(vector<string> {"1", "", "3"}));
	BOOST_CHECK_EQUAL(sv::utils::csvutil::parse_csv_line("1;2;", ';'),
		(vector<string> {"1", "2", ""}));
	BOOST_CHECK_EQUAL(sv::utils::csvutil::parse_csv_line("1;2;3", ';'),
		(vector<string> {"1", "2", "3"}));
	BOOST_CHECK_EQUAL(sv::utils::csvutil::parse_csv_line("\"1\",\"2\",\"3\"", ','),
		(vector<string> {"1", "2", "3"}));
	BOOST_CHECK_EQUAL(sv::utils::csvutil::parse_csv_line("\"1\n1\",\"2\r2\",\"3,3\"", ','),
		(vector<string> {"1\n1", "2\r2", "3,3"}));
	BOOST_CHECK_EQUAL(sv::utils::csvutil::parse_csv_line("\"1\"\"1\",\"2\"\"2\"\"\",\"\"\"3\"\"\"", ','),
		(vector<string> {"1\"1", "2\"2\"", "\"3\""}));
}

BOOST_AUTO_TEST_SUITE_END()
