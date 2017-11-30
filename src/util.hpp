/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <cmath>
#include <map>
#include <string>
#include <vector>

#ifndef Q_MOC_RUN
#include <boost/multiprecision/cpp_dec_float.hpp>
#endif

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QMetaType>
#include <QString>

using std::map;
using std::string;
using std::vector;

namespace sv {
namespace util {

enum class TimeUnit {
	Time = 1,
	Samples = 2
};

enum class SIPrefix {
	unspecified = -1,
	yocto, zepto,
	atto, femto, pico,
	nano, micro, milli,
	none,
	kilo, mega, giga,
	tera, peta, exa,
	zetta, yotta
};

typedef map<const sigrok::Quantity *, QString> quantitymap_t;
typedef map<const sigrok::QuantityFlag *, QString> quantityflagmap_t;
typedef map<const sigrok::Unit *, QString> unitmap_t;

/// Returns the exponent that corresponds to a given prefix.
int exponent(SIPrefix prefix);

/// Timestamp type providing yoctosecond resolution.
typedef boost::multiprecision::number<
	boost::multiprecision::cpp_dec_float<24>,
	boost::multiprecision::et_off> Timestamp;

/**
 * Formats a sigrok quantity to a string
 *
 * @param sr_quantity The sigrok quantity (sigrok::Quantity) to format
 * .
 * @return The formatted quantity.
 */
QString format_quantity(const sigrok::Quantity *sr_quantity);

/**
 * Formats a sigrok quantity flag to a string
 *
 * @param sr_quantityflag The sigrok quantity (sigrok::QuantityFlag) to format
 * .
 * @return The formatted quantity flag.
 */
QString format_quantityflag(const sigrok::QuantityFlag *sr_quantityflag);

/**
 * Formats a sigrok unit to a string
 *
 * @param sr_unit The sigrok unit (sigrok::Unit) to format
 * .
 * @return The formatted unit.
 */
QString format_unit(const sigrok::Unit *sr_unit);

/**
 * Formats a given timestamp with the specified SI prefix.
 *
 * If 'prefix' is left 'unspecified', the function chooses a prefix so that
 * the value in front of the decimal point is between 1 and 999.
 *
 * The default value "s" for the unit argument makes the most sense when
 * formatting time values, but a different value can be given if the function
 * is reused to format a value of another quantity.
 *
 * @param t The value to format.
 * @param prefix The SI prefix to use.
 * @param precision The number of digits after the decimal separator.
 * @param unit The unit of quantity.
 * @param sign Whether or not to add a sign also for positive numbers.
 *
 * @return The formatted value.
 */
QString format_time_si(const Timestamp& v,
	SIPrefix prefix = SIPrefix::unspecified, unsigned precision = 0,
	QString unit = "s", bool sign = true);

/**
 * Wrapper around 'format_time_si()' that interprets the given 'precision'
 * value as the number of decimal places if the timestamp would be formatted
 * without a SI prefix (using 'SIPrefix::none') and adjusts the precision to
 * match the given 'prefix'
 *
 * @param t The value to format.
 * @param prefix The SI prefix to use.
 * @param precision The number of digits after the decimal separator if the
 *        'prefix' would be 'SIPrefix::none', see above for more information.
 * @param unit The unit of quantity.
 * @param sign Whether or not to add a sign also for positive numbers.
 *
 * @return The formatted value.
 */
QString format_time_si_adjusted(const Timestamp& t, SIPrefix prefix,
	unsigned precision = 0, QString unit = "s", bool sign = true);

/**
 * Formats the given timestamp using "[+-]DD:HH:MM:SS.mmm uuu nnn ppp..." format.
 *
 * "DD" and "HH" are left out if they would be "00" (but if "DD" is generated,
 * "HH" is also always generated. The "MM:SS" part is always produced, the
 * number of subsecond digits can be influenced using the 'precision' parameter.
 *
 * @param t The value to format.
 * @param precision The number of digits after the decimal separator.
 * @param sign Whether or not to add a sign also for positive numbers.
 *
 * @return The formatted value.
 */
QString format_time_minutes(const Timestamp& t, signed precision = 0,
	bool sign = true);

/**
 * Split a string into tokens at occurences of the separator.
 *
 * @param[in] text The input string to split.
 * @param[in] separator The delimiter between tokens.
 *
 * @return A vector of broken down tokens.
 */
vector<string> split_string(string text, string separator);


/**
 * Counts the number of digits for the given integer.
 *
 * @param[in] int The integers digits to count.
 *
 * @return Number of digits.
 */
uint count_int_digits(int number);

} // namespace util
} // namespace sv

Q_DECLARE_METATYPE(sv::util::Timestamp)

#endif // UTIL_HPP
