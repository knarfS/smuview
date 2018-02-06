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

#include <algorithm>
#include <cassert>
#include <math.h>
#include <set>
#include <sstream>
#include <vector>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDateTime>
#include <QDebug>
#include <QTextStream>

#include "extdef.h"
#include "util.hpp"

using std::fixed;
using std::max;
using std::ostringstream;
using std::set;
using std::setfill;
using std::setprecision;
using std::showpos;
using std::string;
using std::vector;

namespace sv {
namespace util {

quantity_name_map_t quantity_name_map = {
	{ sigrok::Quantity::VOLTAGE, QString("Voltage") },
	{ sigrok::Quantity::CURRENT, QString("Current") },
	{ sigrok::Quantity::RESISTANCE, QString("Resistance") },
	{ sigrok::Quantity::CAPACITANCE, QString("Capacitance") },
	{ sigrok::Quantity::TEMPERATURE, QString("Temperature") },
	{ sigrok::Quantity::FREQUENCY, QString("Frequency") },
	{ sigrok::Quantity::DUTY_CYCLE, QString("Duty Cycle") },
	{ sigrok::Quantity::CONTINUITY, QString("Continuity") },
	{ sigrok::Quantity::PULSE_WIDTH, QString("Pulse Width") },
	{ sigrok::Quantity::CONDUCTANCE, QString("Conductance") },
	// Electrical power, usually in W, or dBm.
	{ sigrok::Quantity::POWER, QString("Power") },
	// Gain (a transistor's gain, or hFE, for example).
	{ sigrok::Quantity::GAIN, QString("Gain") },
	// Logarithmic representation of sound pressure relative to a reference value.
	{ sigrok::Quantity::SOUND_PRESSURE_LEVEL, QString("Sound Pressure Level") },
	{ sigrok::Quantity::CARBON_MONOXIDE, QString("Carbon Monoxide") },
	{ sigrok::Quantity::RELATIVE_HUMIDITY, QString("Relative Humidity") },
	{ sigrok::Quantity::TIME, QString("Time") },
	{ sigrok::Quantity::WIND_SPEED, QString("Wind Speed") },
	{ sigrok::Quantity::PRESSURE, QString("Pressure") },
	{ sigrok::Quantity::PARALLEL_INDUCTANCE, QString("Parallel Inductance") },
	{ sigrok::Quantity::PARALLEL_CAPACITANCE, QString("Parallel Capacitance") },
	{ sigrok::Quantity::PARALLEL_RESISTANCE, QString("Parallel Resistance") },
	{ sigrok::Quantity::SERIES_INDUCTANCE, QString("Series Inductance") },
	{ sigrok::Quantity::SERIES_CAPACITANCE, QString("Series Capacitance") },
	{ sigrok::Quantity::SERIES_RESISTANCE, QString("Series Resistance") },
	{ sigrok::Quantity::DISSIPATION_FACTOR, QString("Dissipation Factor") },
	{ sigrok::Quantity::QUALITY_FACTOR, QString("Quality Factor") },
	{ sigrok::Quantity::PHASE_ANGLE, QString("Phase Angle") },
	// Difference from reference value.
	{ sigrok::Quantity::DIFFERENCE, QString("Difference") },
	{ sigrok::Quantity::COUNT, QString("Count") },
	{ sigrok::Quantity::POWER_FACTOR, QString("Power Factor") },
	{ sigrok::Quantity::APPARENT_POWER, QString("Apparent Power") },
	{ sigrok::Quantity::MASS, QString("Mass") },
	{ sigrok::Quantity::HARMONIC_RATIO, QString("Harmonic Ratio") },
};

quantity_flag_name_map_t quantity_flag_name_map = {
	{ sigrok::QuantityFlag::AC, QString("AC") },
	{ sigrok::QuantityFlag::DC, QString("DC") },
	{ sigrok::QuantityFlag::RMS, QString("RMS") },
	// Value is voltage drop across a diode, or NAN.
	{ sigrok::QuantityFlag::DIODE, QString("Diode") },
	// Device is in "hold" mode (repeating the last measurement).
	{ sigrok::QuantityFlag::HOLD, QString("Hold") },
	// Device is in "max" mode, only updating upon a new max value.
	{ sigrok::QuantityFlag::MAX, QString("max") },
	// Device is in "min" mode, only updating upon a new min value.
	{ sigrok::QuantityFlag::MIN, QString("min") },
	{ sigrok::QuantityFlag::AUTORANGE, QString("Autorange") },
	{ sigrok::QuantityFlag::RELATIVE, QString("Relative") },
	// Sound pressure level is A-weighted in the frequency domain, according to IEC 61672:2003.
	{ sigrok::QuantityFlag::SPL_FREQ_WEIGHT_A, QString("SPL A-weighted F") },
	// Sound pressure level is C-weighted in the frequency domain, according to IEC 61672:2003.
	{ sigrok::QuantityFlag::SPL_FREQ_WEIGHT_C, QString("SPL C-weighted F") },
	// Sound pressure level is Z-weighted
	{ sigrok::QuantityFlag::SPL_FREQ_WEIGHT_Z, QString("SPL Z-weighted F") },
	// Sound pressure level is not weighted in the frequency domain, albeit without standards-defined low and high frequency limits.
	{ sigrok::QuantityFlag::SPL_FREQ_WEIGHT_FLAT, QString("SPL flat weighted") },
	// Sound pressure level measurement is S-weighted (1s) in the time domain.
	{ sigrok::QuantityFlag::SPL_TIME_WEIGHT_S, QString("SPL S-weighted t") },
	// Sound pressure level measurement is F-weighted (125ms) in the time domain.
	{ sigrok::QuantityFlag::SPL_TIME_WEIGHT_F, QString("SPL F-weighted t") },
	// Sound pressure level is time-averaged (LAT), also known as Equivalent Continuous A-weighted Sound Level (LEQ).
	{ sigrok::QuantityFlag::SPL_LAT, QString("SPL LAT") },
	// Sound pressure level represented as a percentage of measurements that were over a preset alarm level.
	{ sigrok::QuantityFlag::SPL_PCT_OVER_ALARM, QString("SPL Over%") },
	// Time is duration (as opposed to epoch, ...).
	{ sigrok::QuantityFlag::DURATION, QString("Duration") },
	// Device is in "avg" mode, averaging upon each new value.
	{ sigrok::QuantityFlag::AVG, QString("avg") },
	// Reference value shown.
	{ sigrok::QuantityFlag::REFERENCE, QString("Reference") },
	// Unstable value (hasn't settled yet).
	{ sigrok::QuantityFlag::UNSTABLE, QString("Unstable") },
	{ sigrok::QuantityFlag::FOUR_WIRE, QString("4W") }
};

unit_name_map_t unit_name_map = {
	{ sigrok::Unit::VOLT, QString("V") },
	{ sigrok::Unit::AMPERE, QString("A") },
	{ sigrok::Unit::OHM, QString::fromUtf8("\u2126") },
	{ sigrok::Unit::FARAD, QString("F") },
	{ sigrok::Unit::KELVIN, QString("K") },
	{ sigrok::Unit::CELSIUS, QString("°C") },
	{ sigrok::Unit::FAHRENHEIT, QString("°F") },
	{ sigrok::Unit::HERTZ, QString("Hz") },
	{ sigrok::Unit::PERCENTAGE, QString("%") },
	{ sigrok::Unit::BOOLEAN, QString("bool") },
	{ sigrok::Unit::SECOND, QString("s") },
	{ sigrok::Unit::SIEMENS, QString("S") },
	{ sigrok::Unit::DECIBEL_MW, QString("dBm") },
	{ sigrok::Unit::DECIBEL_VOLT, QString("dBV") },
	{ sigrok::Unit::UNITLESS, QString("") },
	{ sigrok::Unit::DECIBEL_SPL, QString("dB") },
	{ sigrok::Unit::CONCENTRATION, QString("ppx") },
	{ sigrok::Unit::REVOLUTIONS_PER_MINUTE, QString("RPM") },
	{ sigrok::Unit::VOLT_AMPERE, QString("VA") },
	{ sigrok::Unit::WATT, QString("W") },
	{ sigrok::Unit::WATT_HOUR, QString("Wh") },
	{ sigrok::Unit::METER_SECOND, QString("m/s") },
	{ sigrok::Unit::HECTOPASCAL, QString("hPa") }, // TODO
	{ sigrok::Unit::HUMIDITY_293K, QString("%") },
	{ sigrok::Unit::DEGREE, QString("°") },
	{ sigrok::Unit::HENRY, QString("H") },
	{ sigrok::Unit::GRAM, QString("g") },
	{ sigrok::Unit::CARAT,QString("ct") },
	{ sigrok::Unit::OUNCE, QString("oz.") },
	{ sigrok::Unit::TROY_OUNCE, QString("oz.tr.") },
	{ sigrok::Unit::POUND, QString("lb") }, // TODO: Pound (lb) or Metric Pound (lbm)?
	{ sigrok::Unit::PENNYWEIGHT, QString("dwt.") },
	{ sigrok::Unit::GRAIN, QString("gr.") },
	{ sigrok::Unit::TAEL, QString::fromUtf8("\u4E24") },
	{ sigrok::Unit::MOMME, QString::fromUtf8("\u5301") },
	{ sigrok::Unit::TOLA, QString("tola") },
	{ sigrok::Unit::PIECE, QString("pc.") }
};

quantity_unit_map_t quantity_unit_map = {
	{ sigrok::Quantity::VOLTAGE, sigrok::Unit::VOLT },
	{ sigrok::Quantity::CURRENT, sigrok::Unit::AMPERE },
	{ sigrok::Quantity::RESISTANCE, sigrok::Unit::OHM },
	{ sigrok::Quantity::CAPACITANCE, sigrok::Unit::FARAD },
	{ sigrok::Quantity::TEMPERATURE, sigrok::Unit::KELVIN },
	{ sigrok::Quantity::FREQUENCY, sigrok::Unit::HERTZ },
	{ sigrok::Quantity::DUTY_CYCLE, sigrok::Unit::PERCENTAGE },
	{ sigrok::Quantity::CONTINUITY, sigrok::Unit::OHM },
	{ sigrok::Quantity::PULSE_WIDTH, sigrok::Unit::PERCENTAGE },
	{ sigrok::Quantity::CONDUCTANCE, sigrok::Unit::SIEMENS },
	{ sigrok::Quantity::POWER, sigrok::Unit::WATT }, // TODO: dBm?
	{ sigrok::Quantity::GAIN, sigrok::Unit::UNITLESS },
	{ sigrok::Quantity::SOUND_PRESSURE_LEVEL, sigrok::Unit::DECIBEL_SPL },
	{ sigrok::Quantity::CARBON_MONOXIDE, sigrok::Unit::CONCENTRATION },
	{ sigrok::Quantity::RELATIVE_HUMIDITY, sigrok::Unit::HUMIDITY_293K },
	{ sigrok::Quantity::TIME, sigrok::Unit::SECOND },
	{ sigrok::Quantity::WIND_SPEED, sigrok::Unit::METER_SECOND },
	{ sigrok::Quantity::PRESSURE, sigrok::Unit::HECTOPASCAL }, // TODO: PASCAL!
	{ sigrok::Quantity::PARALLEL_INDUCTANCE, sigrok::Unit::HENRY },
	{ sigrok::Quantity::PARALLEL_CAPACITANCE, sigrok::Unit::KELVIN },
	{ sigrok::Quantity::PARALLEL_RESISTANCE, sigrok::Unit::OHM },
	{ sigrok::Quantity::SERIES_INDUCTANCE, sigrok::Unit::HENRY },
	{ sigrok::Quantity::SERIES_CAPACITANCE, sigrok::Unit::KELVIN },
	{ sigrok::Quantity::SERIES_RESISTANCE, sigrok::Unit::OHM },
	{ sigrok::Quantity::DISSIPATION_FACTOR, sigrok::Unit::UNITLESS },
	{ sigrok::Quantity::QUALITY_FACTOR, sigrok::Unit::UNITLESS },
	{ sigrok::Quantity::PHASE_ANGLE, sigrok::Unit::DEGREE },
	// Difference from reference value.
	//{ sigrok::Quantity::DIFFERENCE, QString("Difference") },
	{ sigrok::Quantity::COUNT, sigrok::Unit::UNITLESS },
	{ sigrok::Quantity::POWER_FACTOR, sigrok::Unit::UNITLESS },
	{ sigrok::Quantity::APPARENT_POWER, sigrok::Unit::VOLT_AMPERE },
	{ sigrok::Quantity::MASS, sigrok::Unit::GRAM },
	{ sigrok::Quantity::HARMONIC_RATIO, sigrok::Unit::UNITLESS },
};

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

	if (stream.numberFlags() & QTextStream::ForceSign)
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

bool is_valid_sr_quantity(const sigrok::Quantity *sr_quantity)
{
	if (quantity_name_map.count(sr_quantity) > 0)
		return true;
	return false;
}

const sigrok::Unit * get_sr_unit_from_sr_quantity(
	const sigrok::Quantity *sr_quantity)
{
	if (quantity_unit_map.count(sr_quantity) > 0)
		return quantity_unit_map[sr_quantity];
	else {
		qWarning() << "Cannot find a SI Unit for " <<
			format_sr_quantity(sr_quantity);
		assert("Unkown sigrok quantity");
	}

	return nullptr;
}

quantity_name_map_t get_quantity_name_map()
{
	return quantity_name_map;
}

QString format_sr_quantity(const sigrok::Quantity *sr_quantity)
{
	if (quantity_name_map.count(sr_quantity) > 0)
		return quantity_name_map[sr_quantity];
	// TODO: error
	return QString("");
}

quantity_flag_name_map_t get_quantity_flag_name_map()
{
	return quantity_flag_name_map;
}

QString format_sr_quantity_flag(const sigrok::QuantityFlag * sr_quantity_flag)
{
	if (quantity_flag_name_map.count(sr_quantity_flag) > 0)
		return quantity_flag_name_map[sr_quantity_flag];
	// TODO: error
	return QString("");
}

QString format_sr_quantity_flags(
	vector<const sigrok::QuantityFlag *> sr_quantity_flags)
{
	QString quantity_flags("");
	QString sep("");

	// Show AC/DC first
	if (find(sr_quantity_flags.begin(), sr_quantity_flags.end(),
			sigrok::QuantityFlag::AC) != sr_quantity_flags.end()) {
		quantity_flags.append(quantity_flag_name_map[sigrok::QuantityFlag::AC]);
		sep = " ";
	}
	if (find(sr_quantity_flags.begin(), sr_quantity_flags.end(),
			sigrok::QuantityFlag::DC) != sr_quantity_flags.end()) {
		quantity_flags.append(sep);
		quantity_flags.append(quantity_flag_name_map[sigrok::QuantityFlag::DC]);
		sep = " ";
	}
	// 2nd is RMS
	if (find(sr_quantity_flags.begin(), sr_quantity_flags.end(),
			sigrok::QuantityFlag::RMS) != sr_quantity_flags.end()) {
		quantity_flags.append(sep);
		quantity_flags.append(quantity_flag_name_map[sigrok::QuantityFlag::RMS]);
		sep = " ";
	}

	for (auto sr_quantity_flag : sr_quantity_flags) {
		if (sr_quantity_flag == sigrok::QuantityFlag::AC ||
				sr_quantity_flag == sigrok::QuantityFlag::DC ||
				sr_quantity_flag == sigrok::QuantityFlag::RMS)
			continue;

		if (quantity_flag_name_map.count(sr_quantity_flag) == 0)
			continue;

		quantity_flags.append(sep);
		quantity_flags.append(quantity_flag_name_map[sr_quantity_flag]);
		sep = " ";
	}

	return quantity_flags;
}

unit_name_map_t get_unit_name_map()
{
	return unit_name_map;
}

QString format_sr_unit(const sigrok::Unit *sr_unit)
{
	if (unit_name_map.count(sr_unit) > 0)
		return unit_name_map[sr_unit];
	// TODO: error
	return QString("");
}

void format_value_si(
	const double value, const int digits, const int decimal_places,
	QString &value_str, QString &si_prefix_str)
{
	SIPrefix si_prefix;
	if (value == 0)
		si_prefix = SIPrefix::none;
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

	value_str = QString("%1").
		arg(value * multiplier, digits, 'f', decimal_places, QChar(' '));

	QTextStream si_prefix_stream(&si_prefix_str);
	si_prefix_stream << si_prefix;
}

QString format_time_si(const Timestamp& v, SIPrefix prefix,
	unsigned int precision, QString unit, bool sign)
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
		ts << forcesign;
	ts << qSetRealNumberPrecision(precision) << (v * multiplier) << ' '
		<< prefix << unit;

	return s;
}

QString format_time_si_adjusted(const Timestamp& t, SIPrefix prefix,
	unsigned precision, QString unit, bool sign)
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

vector<string> split_string(string text, string separator)
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

uint count_int_digits(int number)
{
	uint n = abs(number);
	int digits = 1;
	while (n >= 10) {
		n /= 10;
		digits++;
	}

	return digits;
}

} // namespace util
} // namespace sv
