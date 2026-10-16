/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_DATAUTIL_HPP
#define DATA_DATAUTIL_HPP

#include <set>
#include <vector>
#include <utility>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QString>

using std::pair;
using std::set;
using std::vector;

namespace sigrok {
class Quantity;
class QuantityFlag;
class Unit;
}

namespace sv {
namespace data {

/**
 * The default value for the total digits used for incomming analog data
 * and for formating values in views, etc.
 */
const int DefaultTotalDigits = 6;

/**
 * The default value for sr_digits for all cases it is not available, e.g. for
 * the math channels
 */
const int DefaultSRDigits = 0;

/**
 * The default value for decimal places for all cases where the decimal places
 * cannot be calculated, e.g. the min/max displays.
 */
const int DefaultDecimalPlaces = 2;

enum class Quantity
{
	Voltage,
	Current,
	Resistance,
	Capacitance,
	Temperature,
	Frequency,
	DutyCyle,
	Continuity,
	PulseWidth,
	Conductance,
	/** Electrical power, usually in W, or dBm. */
	Power,
	ElectricCharge,
	/** Gain (a transistor's gain, or hFE, for example). */
	Gain,
	/**
	 * Logarithmic representation of sound pressure relative to a
	 * reference value.
	 */
	SoundPressureLevel,
	CarbonMonoxide,
	RelativeHumidity,
	Time,
	WindSpeed,
	Pressure,
	ParallelInductance,
	ParallelCapacitance,
	ParallelResistance,
	SeriesInductance,
	SeriesCapacitance,
	SeriesResistance,
	DissipationFactor,
	QualityFactor,
	PhaseAngle,
	/** Difference from reference value. */
	Difference,
	Count,
	PowerFactor,
	ApparentPower,
	Mass,
	HarmonicRatio,
	Energy,
	Unknown,
};

enum class QuantityFlag
{
	AC,
	DC,
	RMS,
	/** Value is voltage drop across a diode, or NAN. */
	Diode,
	/** Device is in "hold" mode (repeating the last measurement). */
	Hold,
	/** Device is in "max" mode, only updating upon a new max value. */
	Max,
	/** Device is in "min" mode, only updating upon a new min value. */
	Min,
	Autorange,
	Relative,
	/**
	 * Sound pressure level is A-weighted in the frequency domain,
	 * according to IEC 61672:2003.
	 */
	SplFreqWeightA,
	/**
	 * Sound pressure level is C-weighted in the frequency domain,
	 * according to IEC 61672:2003.
	 */
	SplFreqWeightC,
	/** Sound pressure level is Z-weighted */
	SplFreqWeightZ,
	/**
	 * Sound pressure level is not weighted in the frequency domain, albeit
	 * without standards-defined low and high frequency limits.
	 */
	SplFreqWeightFlat,
	/**
	 * Sound pressure level measurement is S-weighted (1s) in the time domain.
	 */
	SplTimeWeightS,
	/**
	 * Sound pressure level measurement is F-weighted (125ms) in the
	 * time domain.
	 */
	SplTimeWeightF,
	/**
	 * Sound pressure level is time-averaged (LAT), also known as
	 * Equivalent Continuous A-weighted Sound Level (LEQ).
	 */
	SplLAT,
	/**
	 * Sound pressure level represented as a percentage of measurements that
	 * were over a preset alarm level.
	 */
	SplPctOverAlarm,
	/** Time is duration (as opposed to epoch, ...). */
	Duration,
	/** Device is in "avg" mode, averaging upon each new value. */
	Avg,
	/** Reference value shown. */
	Reference,
	/** Unstable value (hasn't settled yet). */
	Unstable,
	FourWire,
	Unknown,
};

enum class Unit
{
	Volt,
	Ampere,
	Ohm,
	Farad,
	Kelvin,
	Celsius,
	Fahrenheit,
	Hertz,
	Percentage,
	Boolean,
	Second,
	Siemens,
	DecibelMW,
	DecibelVolt,
	Unitless,
	DecibelSpl,
	Concentration,
	RevolutionsPerMinute,
	VoltAmpere,
	Watt,
	WattHour,
	Joule,
	AmpereHour,
	Coulomb,
	MeterPerSecond,
	HectoPascal,
	Humidity293K,
	Degree,
	Henry,
	Gram,
	Carat,
	/** Avoirdupois ounce (oz) */
	Ounce,
	/** Troy ounce (oz t) */
	TroyOunce,
	/** Avoirdupois pound (lb) */
	Pound,
	Pennyweight,
	Grain,
	Tael,
	Momme,
	Tola,
	Piece,
	// TODO: Implement in libsigrok
	//Decibel,
	Unknown,
};

enum class DataType
{
	UInt64,
	String,
	Bool,
	Double,
	RationalPeriod,
	RationalVolt,
	KeyValue,
	UInt64Range,
	DoubleRange,
	Int32,
	MQ,
	UInt32,
	Unknown,
};

typedef pair<Quantity, set<QuantityFlag>> measured_quantity_t;
typedef pair<double, double> double_range_t;
/**
 * Normaly <int64_t, uint64_t> should be used, but <uint64_t, uint64_t>
 * is transfered in the config keys
 */
typedef pair<uint64_t, uint64_t> rational_t;
typedef pair<uint64_t, uint64_t> uint64_range_t;

namespace datautil {

struct QuantityMapping
{
	Quantity quantity;
	const sigrok::Quantity *sr_quantity;
	const QString name;
	set<Unit> units;
};

struct QuantityFlagMapping
{
	QuantityFlag quantity_flag;
	const sigrok::QuantityFlag *sr_quantity_flag;
	const QString name;
};

struct UnitMapping
{
	Unit unit;
	const sigrok::Unit *sr_unit;
	const QString name;
};

struct DataTypeMapping
{
	DataType data_type;
	const sigrok::DataType *sr_data_type;
	const QString name;
};


/**
 * Return all known quantities
 *
 * @return The quantity name map
 */
const vector<QuantityMapping> &get_quantity_map();

/**
 * Return all known quantity flags
 *
 * @return The quantity flags name map
 */
const vector<QuantityFlagMapping> &get_quantity_flag_map();

/**
 * Return all known units
 *
 * @return The unit name map
 */
const vector<UnitMapping> &get_unit_map();

/**
 * Return all known data types
 *
 * @return The data type name map
 */
const vector<DataTypeMapping> &get_data_type_map();


/**
 * Return the corresponding Quantity for a sigrok Quantity
 *
 * @param sr_quantity The sigrok Quantity
 *
 * @return The Quantity.
 */
Quantity get_quantity(const sigrok::Quantity *sr_quantity);

/**
 * Return the corresponding Quantity for a sigrok Quantity (unit32_t)
 *
 * @param sr_quantity The sigrok Quantity as uint32_t
 *
 * @return The Quantity.
 */
Quantity get_quantity(uint32_t sr_quantity);

/**
 * Return the corresponding sigrok Quantity ID for a Quantity
 *
 * @param quantity The Quantity
 *
 * @return The sigrok Quantity ID as uint32_t.
 */
uint32_t get_sr_quantity_id(Quantity quantity);

/**
 * Check if the quantity is a known sigrok quantity
 *
 * @param quantity The quantity
 *
 * @return true if it is a known sigrok quantity
 */
bool is_valid_sr_quantity(data::Quantity quantity);


/**
 * Return the corresponding QuantityFlag for a sigrok QuantityFlag
 *
 * @param sr_quantity_flag The sigrok QuantityFlag
 *
 * @return The QuantityFlag.
 */
QuantityFlag get_quantity_flag(const sigrok::QuantityFlag *sr_quantity_flag);

/**
 * Return the corresponding QuantityFlag for a sigrok QuantityFlag (unit32_t)
 *
 * @param sr_quantity_flag The sigrok QuantityFlag as uint32_t
 *
 * @return The QuantityFlag.
 */
QuantityFlag get_quantity_flag(uint32_t sr_quantity);

/**
 * Return the corresponding sigrok QuantityFlag ID for a QuantityFlag
 *
 * @param quantity_flag The QuantityFlag
 *
 * @return The sigrok QuantityFlag ID as uint64_t.
 */
uint64_t get_sr_quantity_flag_id(QuantityFlag quantity_flag);

/**
 * Return the corresponding QuantityFlags as a set for the
 * sigrok QuantityFlags vector
 *
 * @param sr_quantity_flags The sigrok QuantityFlags as vector
 *
 * @return The QuantityFlags as set.
 */
set<QuantityFlag> get_quantity_flags(
	const vector<const sigrok::QuantityFlag *> &sr_quantity_flags);

/**
 * Return the corresponding QuantityFlags as a set for the
 * sigrok QuantityFlags (uint64_t)
 *
 * @param sr_quantity_flags The sigrok QuantityFlags as uint64_t
 *
 * @return The QuantityFlags as set.
 */
set<QuantityFlag> get_quantity_flags(uint64_t sr_quantity_flags);

/**
 * Return the corresponding QuantityFlags as an uint64_t
 *
 * @param quantity_flags The QuantityFlags as set
 *
 * @return The sigrok QuantityFlags IDs
 */
uint64_t get_sr_quantity_flags_id(const set<QuantityFlag> &quantity_flags);

/**
 * Check if the quantity flag is a known sigrok quantity flag
 *
 * @param quantity_flag The quantity flag
 *
 * @return true if it is a known sigrok quantity flag
 */
bool is_valid_sr_quantity_flag(data::QuantityFlag quantity_flag);


/**
 * Return the corresponding Unit for a sigrok Unit
 *
 * @param sr_unit The sigrok Unit
 *
 * @return The Unit.
 */
Unit get_unit(const sigrok::Unit *sr_unit);

/**
 * Return the corresponding sigrok Unit ID for a Unit
 *
 * @param unit The Unit
 *
 * @return The sigrok Unit ID as uint32_t.
 */
uint32_t get_sr_unit_id(Unit unit);


/**
 * Return the corresponding DataType for a sigrok DataType
 *
 * @param sr_data_type The sigrok DataType
 *
 * @return The DataType.
 */
DataType get_data_type(const sigrok::DataType *sr_data_type);

/**
 * Return the corresponding DataType for a sigrok DataType (unit32_t)
 *
 * @param sr_data_type The sigrok DataType as uint32_t
 *
 * @return The DataType.
 */
DataType get_data_type(uint32_t sr_data_type);

/**
 * Return the corresponding sigrok DataType for a DataType
 *
 * @param data_type The DataType.
 *
 * @return The sigrok DataType.
 */
const sigrok::DataType *get_sr_data_type(DataType data_type);

/**
 * Return the corresponding sigrok DataType ID for a DataType
 *
 * @param data_type The DataType
 *
 * @return The sigrok DataType ID as uint32_t.
 */
uint32_t get_sr_data_type_id(DataType data_type);

/**
 * Check if the DataType is a known sigrok DataType
 *
 * @param data_type The DataType
 *
 * @return true if it is a known sigrok DataType
 */
bool is_valid_sr_data_type(DataType data_type);


/**
 * Format a Quantity to a string
 *
 * @param quantity The Quantity to format.
 *
 * @return The formatted quantity.
 */
QString format_quantity(Quantity quantity);

/**
 * Format a QuantityFlag enum to a string
 *
 * @param quantity_flag The QuantityFlag to format.
 *
 * @return The formatted QuantityFlag.
 */
QString format_quantity_flag(QuantityFlag quantity_flag);

/**
 * Format a QuantityFlag enum set to a string
 *
 * @param quantity_flags The QuantityFlags to format.
 * @param seperator The seperator between the flags.
 *
 * @return The formatted QuantityFlags.
 */
QString format_quantity_flags(const set<QuantityFlag> &quantity_flags,
	const QString &seperator);

/**
 * Format a measured_quantity_t (pair<Quantity, set<QuantityFlag>>) to a string
 *
 * @param measured_quantity The measured_quantity.
 *
 * @return The formated measured_quantity.
 */
QString format_measured_quantity(const measured_quantity_t &measured_quantity);

/**
 * Format a Unit to a string
 *
 * @param unit The quantity to format.
 *
 * @return The formatted unit.
 */
QString format_unit(Unit unit);

/**
 * Format a Unit to a string and adds AC/DC for voltage and current
 *
 * @param unit The quantity to format.
 * @param quantity_flags The quantity flags.
 *
 * @return The formatted unit.
 */
QString format_unit(Unit unit, const set<QuantityFlag> &quantity_flags);

/**
 * Format a DataType to a string
 *
 * @param data_type The DataType to format.
 *
 * @return The formatted DataType.
 */
QString format_data_type(DataType data_type);


/**
 * Return the (SI) units for the given quantity
 *
 * @param quantity The quantity
 *
 * @return The units as set
 */
set<data::Unit> get_units_from_quantity(data::Quantity quantity);

} // namespace datautil
} // namespace data
} // namespace sv

#endif // DATA_DATAUTIL_HPP
