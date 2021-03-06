/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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

static QTextStream& operator<<(QTextStream& stream, SIPrefix prefix)
{
	switch (prefix) {
	case SIPrefix::yocto: return stream << 'y';
	case SIPrefix::zepto: return stream << 'z';
	case SIPrefix::atto:  return stream << 'a';
	case SIPrefix::femto: return stream << 'f';
	case SIPrefix::pico:  return stream << 'p';
	case SIPrefix::nano:  return stream << 'n';
	case SIPrefix::micro: return stream << QChar(0x00B5);
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

// Insert the timestamp value into the stream in fixed-point notation
// (and honor the precision)
static QTextStream& operator<<(QTextStream& stream, const Timestamp& t)
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
		ss << setprecision(1) << round(t);
	else
		ss << setprecision(precision) << t;

	string str(ss.str());
	if (0 == precision) {
		// remove the separator and the unwanted decimal place
		str.resize(str.size() - 2);
	}

	return stream << QString::fromStdString(str);
}

void format_value_si(
	const double value, const int digits, const int decimal_places,
	QString &value_str, QString &si_prefix_str)
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

	// Use actual locale (%L) for formating.
	value_str = QString("%L1").
		arg(value * multiplier, digits, 'f', decimal_places, QChar(' '));

	QTextStream si_prefix_stream(&si_prefix_str);
	si_prefix_stream << si_prefix;
}

QString format_time_si(const Timestamp& v, SIPrefix prefix,
	unsigned int precision, const QString &unit, bool sign)
{
	if (prefix == SIPrefix::unspecified) {
		// No prefix given, calculate it

		if (v.is_zero()) {
			prefix = SIPrefix::none;
		} else {
			int exp = exponent(SIPrefix::yotta);
			prefix = SIPrefix::yocto;
			while ((fabs(v) * pow(Timestamp(10), exp)) > 999 &&
					prefix < SIPrefix::yotta) {
				prefix = successor(prefix);
				exp -= 3;
			}
		}
	}

	assert(prefix >= SIPrefix::yocto);
	assert(prefix <= SIPrefix::yotta);

	const Timestamp multiplier = pow(Timestamp(10), -exponent(prefix));

	QString s;
	QTextStream ts(&s);
	if (sign && !v.is_zero())
		ts.setNumberFlags(ts.numberFlags() | QTextStream::ForceSign);
	ts << qSetRealNumberPrecision((int)precision) << (v * multiplier) << ' '
		<< prefix << unit;

	return s;
}

QString format_time_si_adjusted(const Timestamp& t, SIPrefix prefix,
	unsigned precision, const QString &unit, bool sign)
{
	// The precision is always given without taking the prefix into account
	// so we need to deduct the number of decimals the prefix might imply
	const int prefix_order = -exponent(prefix);

	const unsigned int relative_prec =
		(prefix >= SIPrefix::none) ? precision :
		max((int)(precision - prefix_order), 0);

	return format_time_si(t, prefix, relative_prec, unit, sign);
}

// Helper for 'format_time_minutes()'.
static QString pad_number(unsigned int number, int length)
{
	return QString("%1").arg(number, length, 10, QChar('0'));
}

QString format_time_minutes(const Timestamp& t, signed precision, bool sign)
{
	const Timestamp whole_seconds = floor(abs(t));
	const Timestamp days = floor(whole_seconds / (60 * 60 * 24));
	const unsigned int hours = fmod(whole_seconds / (60 * 60), 24).convert_to<uint>();
	const unsigned int minutes = fmod(whole_seconds / 60, 60).convert_to<uint>();
	const unsigned int seconds = fmod(whole_seconds, 60).convert_to<uint>();

	QString s;
	QTextStream ts(&s);

	if (t < 0)
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

		const Timestamp fraction = fabs(t) - whole_seconds;

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

	return s;
}

QString format_time_date(double t)
{
	QDateTime timestamp;
	timestamp.setMSecsSinceEpoch(t*1000);
	QString date = timestamp.toString("yyyy.MM.dd hh:mm:ss.zzz");
	return date;
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

bool starts_with(const string &str, const string &start_str) {
	return start_str.length() <= str.length() &&
		std::equal(start_str.begin(), start_str.end(), str.begin());
}

uint count_int_digits(int number)
{
	int n = abs(number);
	uint digits = 1;
	while (n >= 10) {
		n /= 10;
		digits++;
	}

	return digits;
}

uint count_double_digits(double max, double step)
{
	int i = (int)floor(max);
	return util::count_int_digits(i) + util::get_decimal_places(step);
}

uint get_decimal_places(double dp)
{
	int d = (int)ceil(1/dp) - 1;
	uint cnt = util::count_int_digits(d);
	return cnt;
}

/*
 * Based on https://stackoverflow.com/a/30338543
 */
vector<string> parse_csv_line(const string &line)
{
	enum State { UnquotedField, QuotedField, QuotedQuote } state = UnquotedField;
	std::vector<std::string> fields {""};

	size_t i = 0; // index of the current field
	for (char c : line) {
		switch (state) {
		case State::UnquotedField:
			switch (c) {
			case ',':
				// end of field
				fields.push_back(""); i++;
				break;
			case '"':
				state = State::QuotedField;
				break;
			default:
				fields[i].push_back(c);
				break;
			}
			break;
		case State::QuotedField:
			switch (c) {
			case '"':
				state = State::QuotedQuote;
				break;
			default:
				fields[i].push_back(c);
				break;
			}
			break;
		case State::QuotedQuote:
			switch (c) {
			case ',':
				// , after closing quote
				fields.push_back(""); i++;
				state = State::UnquotedField;
				break;
			case '"':
				// "" -> "
				fields[i].push_back('"');
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
