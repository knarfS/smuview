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

#include <algorithm>
#include <cassert>
#include <limits>
#include <math.h>
#include <sstream>
#include <vector>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDateTime>
#include <QDebug>
#include <QTextStream>
#include <QUuid>

#include "extdef.h"
#include "util.hpp"

using std::fixed;
using std::max;
using std::ostringstream;
using std::setfill;
using std::setprecision;
using std::showpos;
using std::string;
using std::vector;

namespace sv {
namespace util {

static QTextStream &operator<<(QTextStream &stream, SIPrefix prefix)
{
	switch (prefix) {
	case SIPrefix::yocto: return stream << 'y';
	case SIPrefix::zepto: return stream << 'z';
	case SIPrefix::atto:  return stream << 'a';
	case SIPrefix::femto: return stream << 'f';
	case SIPrefix::pico:  return stream << 'p';
	case SIPrefix::nano:  return stream << 'n';
	case SIPrefix::micro: return stream << QChar(0x03BC);
	case SIPrefix::milli: return stream << 'm';
	case SIPrefix::kilo:  return stream << 'k';
	case SIPrefix::mega:  return stream << 'M';
	case SIPrefix::giga:  return stream << 'G';
	case SIPrefix::tera:  return stream << 'T';
	case SIPrefix::peta:  return stream << 'P';
	case SIPrefix::exa:   return stream << 'E';
	case SIPrefix::zetta: return stream << 'Z';
	case SIPrefix::yotta: return stream << 'Y';

	default: return stream;
	}
}

int exponent(SIPrefix prefix)
{
	return 3 * (static_cast<int>(prefix) - static_cast<int>(SIPrefix::none));
}

static SIPrefix successor(SIPrefix prefix)
{
	assert(prefix != SIPrefix::yotta);
	return static_cast<SIPrefix>(static_cast<int>(prefix) + 1);
}

static int prefix_from_si_prefix(SIPrefix prefix)
{
	return static_cast<int>(prefix) - static_cast<int>(SIPrefix::none);
}

static SIPrefix si_prefix_from_prefix(int prefix)
{
	return static_cast<SIPrefix>(static_cast<int>(SIPrefix::none) + prefix);
}

// Insert the timestamp value into the stream in fixed-point notation
// (and honor the precision)
static QTextStream &operator<<(QTextStream &stream, const Timestamp &timestamp)
{
	// The multiprecision types already have a function and a stream insertion
	// operator to convert them to a string, however these functions abuse a
	// precision value of zero to print all available decimal places instead of
	// none, and the boost authors refuse to fix this because they don't want
	// to break buggy code that relies on this bug.
	// (https://svn.boost.org/trac/boost/ticket/10103)
	// Therefore we have to work around the case where precision is zero.

	int precision = stream.realNumberPrecision();

	ostringstream ss;
	ss << fixed;

	if ((stream.numberFlags() & QTextStream::ForceSign) != 0)
		ss << showpos;

	if (0 == precision)
		ss << setprecision(1) << round(timestamp);
	else
		ss << setprecision(precision) << timestamp;

	string str(ss.str());
	if (0 == precision) {
		// remove the separator and the unwanted decimal place
		str.resize(str.size() - 2);
	}

	return stream << QString::fromStdString(str);
}

int prefix_from_value(const double value, const int sr_digits)
{
	double logval = log10(fabs(value));
	int prefix = static_cast<int>((logval / 3) - static_cast<int>(logval < 1));

	if (value == 0 || value == NAN ||
			value == std::numeric_limits<double>::infinity() ||
			value >= std::numeric_limits<double>::max() ||
			value <= std::numeric_limits<double>::lowest()) {
		prefix = prefix_from_si_prefix(SIPrefix::none);
	}
	else if (prefix < prefix_from_si_prefix(SIPrefix::yocto))
		prefix = prefix_from_si_prefix(SIPrefix::yocto);
	else if (prefix > prefix_from_si_prefix(SIPrefix::yotta))
		prefix = prefix_from_si_prefix(SIPrefix::yotta);
	else if (3 * prefix < -sr_digits)
		prefix = (-sr_digits + 2 * static_cast<int>(sr_digits < 0)) / 3;

	return prefix;
}

int decimal_places_from_prefix(const int prefix, const int sr_digits)
{
	int decimal_places = sr_digits + (3 * prefix);
	decimal_places = decimal_places < 0 ? 0 : decimal_places;

	return decimal_places;
}

void format_value_si(
	const double value, const int total_digits, const int sr_digits,
	QString &value_str, QString &si_prefix_str, const bool use_locale)
{
	int prefix = prefix_from_value(value, sr_digits);
	SIPrefix si_prefix = si_prefix_from_prefix(prefix);
	assert(si_prefix >= SIPrefix::yocto);
	assert(si_prefix <= SIPrefix::yotta);

	int decimal_places = decimal_places_from_prefix(prefix, sr_digits);

	double new_value = value * pow(10, -3 * prefix);

	// Check if, use current locale (%L) for formating.
	QString format_string = use_locale ? "%L1" : "%1";
	value_str = QString(format_string)
		.arg(new_value, total_digits, 'f', decimal_places, QChar(' '));

	QTextStream si_prefix_stream(&si_prefix_str);
	si_prefix_stream << si_prefix;
}

void format_value_si_autoscale(
	const double value, const int total_digits, const int decimal_places,
	QString &value_str, QString &si_prefix_str, const bool use_locale)
{
	SIPrefix si_prefix;
	if (value == 0 || value == NAN ||
			value == std::numeric_limits<double>::infinity() ||
			value >= std::numeric_limits<double>::max() ||
			value <= std::numeric_limits<double>::lowest()) {
		si_prefix = SIPrefix::none;
	}
	else {
		int exp = exponent(SIPrefix::yotta);
		si_prefix = SIPrefix::yocto;
		while ((fabs(value) * pow(10, exp)) > 999 &&
				si_prefix < SIPrefix::yotta) {
			si_prefix = successor(si_prefix);
			exp -= 3;
		}
	}
	assert(si_prefix >= SIPrefix::yocto);
	assert(si_prefix <= SIPrefix::yotta);

	const double multiplier = pow(10, -exponent(si_prefix));

	// Check if, use current locale (%L) for formating.
	QString format_string = use_locale ? "%L1" : "%1";
	value_str = QString(format_string).
		arg(value * multiplier, total_digits, 'f', decimal_places, QChar(' '));

	QTextStream si_prefix_stream(&si_prefix_str);
	si_prefix_stream << si_prefix;
}

QString format_time_si(const Timestamp &timestamp, SIPrefix prefix,
	unsigned int precision, const QString &unit, bool sign)
{
	if (prefix == SIPrefix::unspecified) {
		// No prefix given, calculate it

		if (timestamp.is_zero()) {
			prefix = SIPrefix::none;
		}
		else {
			int exp = exponent(SIPrefix::yotta);
			prefix = SIPrefix::yocto;
			while ((fabs(timestamp) * pow(Timestamp(10), exp)) > 999 &&
					prefix < SIPrefix::yotta) {
				prefix = successor(prefix);
				exp -= 3;
			}
		}
	}

	assert(prefix >= SIPrefix::yocto);
	assert(prefix <= SIPrefix::yotta);

	const Timestamp multiplier = pow(Timestamp(10), -exponent(prefix));

	QString str;
	QTextStream ts(&str);
	if (sign && !timestamp.is_zero())
		ts.setNumberFlags(ts.numberFlags() | QTextStream::ForceSign);
	ts << qSetRealNumberPrecision((int)precision) << (timestamp * multiplier)
		<< ' ' << prefix << unit;

	return str;
}

QString format_time_si_adjusted(const Timestamp &timestamp, SIPrefix prefix,
	unsigned precision, const QString &unit, bool sign)
{
	// The precision is always given without taking the prefix into account
	// so we need to deduct the number of decimals the prefix might imply
	const int prefix_order = -exponent(prefix);

	const unsigned int relative_prec = (prefix >= SIPrefix::none) ?
		precision : max((int)(precision - prefix_order), 0);

	return format_time_si(timestamp, prefix, relative_prec, unit, sign);
}

// Helper for 'format_time_minutes()'.
static QString pad_number(unsigned int number, int length)
{
	return QString("%1").arg(number, length, 10, QChar('0'));
}

QString format_time_minutes(const Timestamp &timestamp, signed precision,
	bool sign)
{
	const Timestamp whole_seconds = floor(abs(timestamp));
	const Timestamp days = floor(whole_seconds / (60 * 60 * 24));
	const unsigned int hours = fmod(whole_seconds / (60 * 60), 24).convert_to<uint>();
	const unsigned int minutes = fmod(whole_seconds / 60, 60).convert_to<uint>();
	const unsigned int seconds = fmod(whole_seconds, 60).convert_to<uint>();

	QString str;
	QTextStream ts(&str);

	if (timestamp < 0)
		ts << "-";
	else if (sign)
		ts << "+";

	bool use_padding = false;

	// DD
	if (days) {
		ts << days.str().c_str() << ":";
		use_padding = true;
	}

	// HH
	if (hours || days) {
		ts << pad_number(hours, use_padding ? 2 : 0) << ":";
		use_padding = true;
	}

	// MM
	ts << pad_number(minutes, use_padding ? 2 : 0);

	ts << ":";

	// SS
	ts << pad_number(seconds, 2);

	if (precision) {
		ts << ".";

		const Timestamp fraction = fabs(timestamp) - whole_seconds;

		ostringstream ss;
		ss << fixed << setprecision(precision) << setfill('0') << fraction;
		string fs = ss.str();

		// Copy all digits, inserting spaces as unit separators
		for (int i = 1; i <= precision; i++) {
			// Start at index 2 to skip the "0." at the beginning
			ts << fs.at(1 + i);

			if ((i > 0) && (i % 3 == 0) && (i != precision))
				ts << " ";
		}
	}

	return str;
}

QString format_time_date(double timestamp)
{
	QDateTime date;
	date.setMSecsSinceEpoch(static_cast<qint64>(timestamp * 1000));
	return date.toString("yyyy.MM.dd hh:mm:ss.zzz");
}

string format_uuid(QUuid uuid)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	return uuid.toString(QUuid::WithoutBraces).toStdString();
#else
	return uuid.toString().replace("{", "").replace("}", "").toStdString();
#endif
}

vector<string> split_string(string text, const string &separator)
{
	vector<string> result;
	size_t pos;

	while ((pos = text.find(separator)) != std::string::npos) {
		result.push_back(text.substr(0, pos));
		text = text.substr(pos + separator.length());
	}
	result.push_back(text);

	return result;
}

bool starts_with(const string &str, const string &start_str)
{
	return start_str.length() <= str.length() &&
		std::equal(start_str.begin(), start_str.end(), str.begin());
}

int count_int_digits(int number)
{
	if (number == 0)
		return 0;
	int abs_number = abs(number);
	int digits = 1;
	while (abs_number >= 10) {
		abs_number /= 10;
		digits++;
	}
	return digits;
}

int count_double_digits(double max_value, double step)
{
	int count_int = util::count_int_digits((int)floor(max_value));
	int count_frac = util::count_decimal_places(fmod(max_value, 1.0));
	int count_step = util::count_decimal_places(step);
	return count_int + (count_frac > count_step ? count_frac : count_step);
}

int count_decimal_places(double step)
{
	double frac_part = fmod(step, 1.0);
	if (frac_part == 0)
		return 0;

	std::stringstream stream;
	stream << frac_part;
	std::string frac_str = stream.str();

	// Check for exponential notation
	size_t e_pos = frac_str.find('e');
	if (e_pos != std::string::npos) {
		std::string exponent_str = frac_str.substr(e_pos + 1);
		return -(std::stoi(exponent_str));
	}

	// Check for the decimal point
	size_t point_pos = frac_str.find('.');
	if (point_pos != std::string::npos) {
		return static_cast<int>(frac_str.length() - point_pos - 1);
	}

	return 0;

	/*
	* Old implementation, didn't worked for e.g. `0.111`
	double frac_part = fmod(step, 1.0);
	if (frac_part == 0)
			return 0;
	int decimal = (int)floor(1/frac_part) - 1;
	return util::count_int_digits(decimal);
	*/
}

int get_sr_digits(double step)
{
	if (step == 0)
		return 0;

	int count_frac = util::count_decimal_places(fmod(step, 1.0));
	if (count_frac > 0)
		return count_frac;

	// Count the zeros at the end of the integer part
	std::stringstream stream;
	stream << static_cast<int>(floor(step));
	std::string int_str = stream.str();

	int int_count = 0;
	size_t int_str_pos = int_str.length();
	while (int_str_pos > 0) {
		int_str_pos--;
		if (int_str[int_str_pos] != '0')
			break;
		int_count++;
	}

	return -int_count;
}

/*
 * Based on https://stackoverflow.com/a/30338543
 */
vector<string> parse_csv_line(const string &line)
{
	enum State { UnquotedField, QuotedField, QuotedQuote } state = UnquotedField;
	std::vector<std::string> fields{""};

	size_t index = 0; // index of the current field
	for (char chr : line) {
		switch (state) {
		case State::UnquotedField:
			switch (chr) {
			case ',':
				// end of field
				fields.push_back("");
				index++;
				break;
			case '"':
				state = State::QuotedField;
				break;
			default:
				fields[index].push_back(chr);
				break;
			}
			break;
		case State::QuotedField:
			switch (chr) {
			case '"':
				state = State::QuotedQuote;
				break;
			default:
				fields[index].push_back(chr);
				break;
			}
			break;
		case State::QuotedQuote:
			switch (chr) {
			case ',':
				// , after closing quote
				fields.push_back("");
				index++;
				state = State::UnquotedField;
				break;
			case '"':
				// "" -> "
				fields[index].push_back('"');
				state = State::QuotedField;
				break;
			default:
				// end of quote
				state = State::UnquotedField;
				break;
			}
			break;
		}
	}

	return fields;
}

} // namespace util
} // namespace sv
