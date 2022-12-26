/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017-2022 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifndef Q_MOC_RUN
#include <boost/serialization/nvp.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#endif

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QMetaType>
#include <QString>
#include <QUuid>

using std::map;
using std::set;
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

/// Returns the exponent that corresponds to a given prefix.
int exponent(SIPrefix prefix);

/// Timestamp type providing yoctosecond resolution.
typedef boost::multiprecision::number<
	boost::multiprecision::cpp_dec_float<24>,
	boost::multiprecision::et_off> Timestamp;

/**
 * Returns the SI prefix as `int` based on the `value` and the sigrok digits.
 *
 * @param value The value.
 * @param sr_digits The digits from the sigrok analog payload. Something like
 *                  exponent with reversed polarity.
 *
 * TODO: move to data
 */
int prefix_from_value(const double value, const int sr_digits);

/**
 * Returns the number of decimal places based on the `prefix` and the sigrok digits.
 *
 * @param prefix The SI prefix as an `int`.
 * @param sr_digits The digits from the sigrok analog payload. Something like
 *                  exponent with reversed polarity.
 *
 * TODO: move to data
 */
int decimal_places_from_prefix(const int prefix, const int sr_digits);

/**
 * Formats and rescales a given double value and stores the results in
 * `value_str` and `si_prefix`.
 *
 * @param value The value to format.
 * @param total_digits The number of total digits (incl. the decimal places)
 *                     for `value_str`. This is directly passed to
 *                     `QString.arg()` as `fieldWidth`.
 * @param sr_digits The digits from the sigrok analog payload. Something like
 *                  exponent with reversed polarity.
 * @param value_str A reference to a `QString` to stre the digits to.
 * @param si_prefix A reference to a `QString` to store the SI prefix to.
 * @param use_locale Format `value_str` by using the locale settings, otherwise
 *                   the "C" locale will be used.
 *
 * TODO: move to data
 */
void format_value_si(
	const double value, const int total_digits, const int sr_digits,
	QString &value_str, QString &si_prefix_str, const bool use_locale = true);

void format_value_si_autoscale(
	const double value, const int total_digits, const int decimal_places,
	QString &value_str, QString &si_prefix_str, const bool use_locale = true);

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
 * @param timestamp The value to format.
 * @param prefix The SI prefix to use.
 * @param precision The number of digits after the decimal separator.
 * @param unit The unit of quantity.
 * @param sign Whether or not to add a sign also for positive numbers.
 *
 * @return The formatted value.
 *
 * TODO: move to data
 */
QString format_time_si(const Timestamp& timestamp,
	SIPrefix prefix = SIPrefix::unspecified, unsigned precision = 0,
	const QString &unit = "s", bool sign = true);

/**
 * Wrapper around 'format_time_si()' that interprets the given 'precision'
 * value as the number of decimal places if the timestamp would be formatted
 * without a SI prefix (using 'SIPrefix::none') and adjusts the precision to
 * match the given 'prefix'
 *
 * @param timestamp The value to format.
 * @param prefix The SI prefix to use.
 * @param precision The number of digits after the decimal separator if the
 *        'prefix' would be 'SIPrefix::none', see above for more information.
 * @param unit The unit of quantity.
 * @param sign Whether or not to add a sign also for positive numbers.
 *
 * @return The formatted value.
 *
 * TODO: move to data
 */
QString format_time_si_adjusted(const Timestamp& timestamp, SIPrefix prefix,
	unsigned precision = 0, const QString &unit = "s", bool sign = true);

/**
 * Formats the given timestamp using "[+-]DD:HH:MM:SS.mmm uuu nnn ppp..." format.
 *
 * "DD" and "HH" are left out if they would be "00" (but if "DD" is generated,
 * "HH" is also always generated. The "MM:SS" part is always produced, the
 * number of subsecond digits can be influenced using the 'precision' parameter.
 *
 * @param timestamp The value to format.
 * @param precision The number of digits after the decimal separator.
 * @param sign Whether or not to add a sign also for positive numbers.
 *
 * @return The formatted value.
 *
 * TODO: move to data
 */
QString format_time_minutes(const Timestamp& timestamp, signed precision = 0,
	bool sign = true);

/**
 * Formats the given timestamp as a date using
 * "yyyy.MM.dd hh:mm:ss.zzz" QDateTime.toString() format.
 *
 * The number of subsecond digits can be influenced using the
 * 'precision' parameter.
 *
 * @param timestamp The value to format.
 * @param precision The number of digits after the decimal separator.
 *
 * @return The formatted date.
 *
 * TODO: move to data
 */
QString format_time_date(double timestamp);

/**
 * Format the given UUID as a string without braches.
 *
 * @param uuid The UUID to format.
 *
 * @return The formated UUID.
 */
string format_uuid(QUuid uuid);

/**
 * Split a string into tokens at occurences of the separator.
 *
 * @param[in] text The input string to split.
 * @param[in] separator The delimiter between tokens.
 *
 * @return A vector of broken down tokens.
 */
vector<string> split_string(string text, const string &separator);

/**
 * Check if a string 'str' starts with the string 'start_str'.
 *
 * @param[in] str The string to check.
 * @param[in] start_str The start string.
 *
 * @return True if string str starts with string start_str.
 */
bool starts_with(const string &str, const string &start_str);

/**
 * Counts the number of digits for the given integer.
 *
 * @param[in] int The integers digits to count.
 *
 * @return Number of total digits.
 */
int count_int_digits(int number);

/**
 * Get the number of digits for the given double.
 *
 * @param[in] value The value of the double.
 * @param[in] step Step size of the double.
 *
 * @return Number of total digits
 */
int count_double_digits(double value, double step);

/**
 * Count the number of decimal places (number of digits after the decimal point)
 *
 * @param[in] step The step size from which to calculate the decimal places
 *
 * @return Number of decimal places
 */
int count_decimal_places(double step);


/**
 * Get the sr_digits as used in the analog payload from the step size.
 *
 * @param[in] step The step size from which to calculate the decimal places
 *
 * @return The sr_digits
 */
int get_sr_digits(double step);

/**
 * Parse a single CSV line.
 * Based on https://stackoverflow.com/a/30338543
 *
 * @param[in] line The CSV line to parse.
 *
 * @return A vector of the values.
 */
vector<string> parse_csv_line(const string &line);

} // namespace util
} // namespace sv

Q_DECLARE_METATYPE(sv::util::Timestamp)

#endif // UTIL_HPP
