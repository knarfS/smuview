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

#ifndef SMUVIEW_TEST_TESTUTIL_BOOSTUTILS_HPP
#define SMUVIEW_TEST_TESTUTIL_BOOSTUTILS_HPP

#include <map>
#include <optional>
#include <ostream>
#include <utility>
#include <vector>
#include <boost/test/tools/detail/print_helper.hpp>
#include <boost/test/unit_test.hpp>

#include <QString>

namespace boost::test_tools::tt_detail {

template<typename T>
struct print_log_value<std::vector<T>>
{
	void operator()(std::ostream &os, const std::vector<T> &v)
	{
		std::string sep("");
		os << "[";
		for (const T &e : v) {
			os << sep;
			print_log_value<T>()(os, e);
			sep = ", ";
		}
		os << "]";
	}
};

template<typename T, typename U>
struct print_log_value<std::map<T, U>>
{
	void operator()(std::ostream &os, const std::map<T, U> &m)
	{
		std::string sep("");
		os << "{";
		for (const auto &p : m) {
			os << sep << "(";
			print_log_value<T>()(os, p.first);
			os << ", ";
			print_log_value<U>()(os, p.second);
			os << ")";
			sep = ", ";
		}
		os << "}";
	}
};

template<typename T, typename U>
struct print_log_value<std::pair<T, U>>
{
	void operator()(std::ostream &os, const std::pair<T, U> &p)
	{
		os << "(";
		print_log_value<T>()(os, p.first);
		os << ", ";
		print_log_value<U>()(os, p.second);
		os << ")";
	}
};

template<typename T>
struct print_log_value<std::optional<T>>
{
	void operator()(std::ostream &os, const std::optional<T> &o)
	{
		if (!o.has_value())
			os << "std::nullopt";
		else
			print_log_value<T>()(os, o.value());
	}
};

template<> struct print_log_value<QString>
{
	void operator()(std::ostream &os, const QString &s)
	{
		os << s.toStdString();
	}
};

} // namespace boost::test_tools::tt_detail

#endif // SMUVIEW_TEST_TESTUTIL_BOOSTUTILS_HPP
