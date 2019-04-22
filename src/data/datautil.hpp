/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_DATAUTIL_HPP
#define DATA_DATAUTIL_HPP

#include <map>
#include <set>
#include <vector>
#include <utility>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QObject>
#include <QString>

using std::map;
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
	/** Also "Energy". TODO: Implement in libsigrok */
	Work,
	/** TODO: Implement in libsigrok */
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
	/** TODO: Implement in libsigrok */
	Decibel,
	Unitless,
	DecibelSpl,
	Concentration,
	RevolutionsPerMinute,
	VoltAmpere,
	Watt,
	WattHour,
	/** TODO: Implement in libsigrok */
	Joule,
	/** TODO: Implement in libsigrok */
	AmpereHour,
	/** TODO: Implement in libsigrok */
	Coulomb,
	MeterPerSecond,
	/** TODO: Use Pascal instead? */
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
	Uint64Range,
	DoubleRange,
	Int32,
	MQ,
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

typedef map<Quantity, QString> quantity_name_map_t;
typedef map<QuantityFlag, QString> quantity_flag_name_map_t;
typedef map<Unit, QString> unit_name_map_t;
typedef map<DataType, QString> data_type_name_map_t;

namespace {

typedef map<Quantity, set<Unit>> quantity_unit_map_t;

// TODO: Use tr(), QCoreApplication::translate(), QT_TR_NOOP() or
//       QT_TRANSLATE_NOOP() for translation.
//       See: http://doc.qt.io/qt-5/i18n-source-translation.html
quantity_name_map_t quantity_name_map = {
	{ Quantity::Voltage, QString("Voltage") },
	{ Quantity::Current, QString("Current") },
	{ Quantity::Resistance, QString("Resistance") },
	{ Quantity::Capacitance, QString("Capacitance") },
	{ Quantity::Temperature, QString("Temperature") },
	{ Quantity::Frequency, QString("Frequency") },
	{ Quantity::DutyCyle, QString("Duty Cycle") },
	{ Quantity::Continuity, QString("Continuity") },
	{ Quantity::PulseWidth, QString("Pulse Width") },
	{ Quantity::Conductance, QString("Conductance") },
	{ Quantity::Power, QString("Power") },
	{ Quantity::Work, QString("Work") },
	{ Quantity::ElectricCharge, QString("Electric Charge") },
	{ Quantity::Gain, QString("Gain") },
	{ Quantity::SoundPressureLevel, QString("Sound Pressure Level") },
	{ Quantity::CarbonMonoxide, QString("Carbon Monoxide") },
	{ Quantity::RelativeHumidity, QString("Relative Humidity") },
	{ Quantity::Time, QString("Time") },
	{ Quantity::WindSpeed, QString("Wind Speed") },
	{ Quantity::Pressure, QString("Pressure") },
	{ Quantity::ParallelInductance, QString("Parallel Inductance") },
	{ Quantity::ParallelCapacitance, QString("Parallel Capacitance") },
	{ Quantity::ParallelResistance, QString("Parallel Resistance") },
	{ Quantity::SeriesInductance, QString("Series Inductance") },
	{ Quantity::SeriesCapacitance, QString("Series Capacitance") },
	{ Quantity::SeriesResistance, QString("Series Resistance") },
	{ Quantity::DissipationFactor, QString("Dissipation Factor") },
	{ Quantity::QualityFactor, QString("Quality Factor") },
	{ Quantity::PhaseAngle, QString("Phase Angle") },
	{ Quantity::Difference, QString("Difference") },
	{ Quantity::Count, QString("Count") },
	{ Quantity::PowerFactor, QString("Power Factor") },
	{ Quantity::ApparentPower, QString("Apparent Power") },
	{ Quantity::Mass, QString("Mass") },
	{ Quantity::HarmonicRatio, QString("Harmonic Ratio") },
	{ Quantity::Unknown, QString("Unknown") },
};

quantity_flag_name_map_t quantity_flag_name_map = {
	{ QuantityFlag::AC, QString("AC") },
	{ QuantityFlag::DC, QString("DC") },
	{ QuantityFlag::RMS, QString("RMS") },
	{ QuantityFlag::Diode, QString("Diode") },
	{ QuantityFlag::Hold, QString("Hold") },
	{ QuantityFlag::Max, QString("max") },
	{ QuantityFlag::Min, QString("min") },
	{ QuantityFlag::Autorange, QString("Autorange") },
	{ QuantityFlag::Relative, QString("Relative") },
	{ QuantityFlag::SplFreqWeightA, QString("SPL A-weighted F") },
	{ QuantityFlag::SplFreqWeightC, QString("SPL C-weighted F") },
	{ QuantityFlag::SplFreqWeightZ, QString("SPL Z-weighted F") },
	{ QuantityFlag::SplFreqWeightFlat, QString("SPL flat weighted") },
	{ QuantityFlag::SplTimeWeightS, QString("SPL S-weighted t") },
	{ QuantityFlag::SplTimeWeightF, QString("SPL F-weighted t") },
	{ QuantityFlag::SplLAT, QString("SPL LAT") },
	{ QuantityFlag::SplPctOverAlarm, QString("SPL Over%") },
	{ QuantityFlag::Duration, QString("Duration") },
	{ QuantityFlag::Avg, QString("avg") },
	{ QuantityFlag::Reference, QString("Reference") },
	{ QuantityFlag::Unstable, QString("Unstable") },
	{ QuantityFlag::FourWire, QString("4W") },
	{ QuantityFlag::Unknown, QString("Unknown") },
};

unit_name_map_t unit_name_map = {
	{ Unit::Volt, QString("V") },
	{ Unit::Ampere, QString("A") },
	{ Unit::Ohm, QString::fromUtf8("\u2126") },
	{ Unit::Farad, QString("F") },
	{ Unit::Kelvin, QString("K") },
	{ Unit::Celsius, QString("°C") },
	{ Unit::Fahrenheit, QString("°F") },
	{ Unit::Hertz, QString("Hz") },
	{ Unit::Percentage, QString("%") },
	{ Unit::Boolean, QString("bool") },
	{ Unit::Second, QString("s") },
	{ Unit::Siemens, QString("S") },
	{ Unit::DecibelMW, QString("dBm") },
	{ Unit::DecibelVolt, QString("dBV") },
	{ Unit::Decibel, QString("dB") },
	{ Unit::Unitless, QString("") },
	{ Unit::DecibelSpl, QString("dB") },
	{ Unit::Concentration, QString("ppx") },
	{ Unit::RevolutionsPerMinute, QString("RPM") },
	{ Unit::VoltAmpere, QString("VA") },
	{ Unit::Watt, QString("W") },
	{ Unit::WattHour, QString("Wh") },
	{ Unit::Joule, QString("J") },
	{ Unit::AmpereHour, QString("Ah") },
	{ Unit::Coulomb, QString("C") },
	{ Unit::MeterPerSecond, QString("m/s") },
	{ Unit::HectoPascal, QString("hPa") },
	{ Unit::Humidity293K, QString("%") },
	{ Unit::Degree, QString("°") },
	{ Unit::Henry, QString("H") },
	{ Unit::Gram, QString("g") },
	{ Unit::Carat, QString("ct") },
	{ Unit::Ounce, QString("oz.") },
	{ Unit::TroyOunce, QString("oz.tr.") },
	{ Unit::Pound, QString("lb") },
	{ Unit::Pennyweight, QString("dwt.") },
	{ Unit::Grain, QString("gr.") },
	{ Unit::Tael, QString::fromUtf8("\u4E24") },
	{ Unit::Momme, QString::fromUtf8("\u5301") },
	{ Unit::Tola, QString("tola") },
	{ Unit::Piece, QString("pc.") },
	{ Unit::Unknown, QString("??") },
};

data_type_name_map_t data_type_name_map = {
	{ DataType::UInt64, QString("UInt64") },
	{ DataType::String, QString("String") },
	{ DataType::Bool, QString("Boolean") },
	{ DataType::Double, QString("Double") },
	{ DataType::RationalPeriod, QString("Rational Period") },
	{ DataType::RationalVolt, QString("Rational Volt") },
	{ DataType::KeyValue, QString("Key Value") },
	{ DataType::Uint64Range, QString("Uint64 Range") },
	{ DataType::DoubleRange, QString("Double Range") },
	{ DataType::Int32, QString("Int32") },
	{ DataType::MQ, QString("Measured Quantity") },
	{ DataType::Unknown, QString("Unknown") },
};

map<const sigrok::Quantity *, Quantity> sr_quantity_quantity_map = {
	{ sigrok::Quantity::VOLTAGE, Quantity::Voltage },
	{ sigrok::Quantity::CURRENT, Quantity::Current },
	{ sigrok::Quantity::RESISTANCE, Quantity::Resistance },
	{ sigrok::Quantity::CAPACITANCE, Quantity::Capacitance },
	{ sigrok::Quantity::TEMPERATURE, Quantity::Temperature },
	{ sigrok::Quantity::FREQUENCY, Quantity::Frequency },
	{ sigrok::Quantity::DUTY_CYCLE, Quantity::DutyCyle },
	{ sigrok::Quantity::CONTINUITY, Quantity::Continuity },
	{ sigrok::Quantity::PULSE_WIDTH, Quantity::PulseWidth },
	{ sigrok::Quantity::CONDUCTANCE, Quantity::Conductance },
	{ sigrok::Quantity::POWER, Quantity::Power },
	{ sigrok::Quantity::GAIN, Quantity::Gain },
	{ sigrok::Quantity::SOUND_PRESSURE_LEVEL, Quantity::SoundPressureLevel },
	{ sigrok::Quantity::CARBON_MONOXIDE, Quantity::CarbonMonoxide },
	{ sigrok::Quantity::RELATIVE_HUMIDITY, Quantity::RelativeHumidity },
	{ sigrok::Quantity::TIME, Quantity::Time },
	{ sigrok::Quantity::WIND_SPEED, Quantity::WindSpeed },
	{ sigrok::Quantity::PRESSURE, Quantity::Pressure },
	{ sigrok::Quantity::PARALLEL_INDUCTANCE, Quantity::ParallelInductance },
	{ sigrok::Quantity::PARALLEL_CAPACITANCE, Quantity::ParallelCapacitance },
	{ sigrok::Quantity::PARALLEL_RESISTANCE, Quantity::ParallelResistance },
	{ sigrok::Quantity::SERIES_INDUCTANCE, Quantity::SeriesInductance },
	{ sigrok::Quantity::SERIES_CAPACITANCE, Quantity::SeriesCapacitance },
	{ sigrok::Quantity::SERIES_RESISTANCE, Quantity::SeriesResistance },
	{ sigrok::Quantity::DISSIPATION_FACTOR, Quantity::DissipationFactor },
	{ sigrok::Quantity::QUALITY_FACTOR, Quantity::QualityFactor },
	{ sigrok::Quantity::PHASE_ANGLE, Quantity::PhaseAngle },
	{ sigrok::Quantity::DIFFERENCE, Quantity::Difference },
	{ sigrok::Quantity::COUNT, Quantity::Count },
	{ sigrok::Quantity::POWER_FACTOR, Quantity::PowerFactor },
	{ sigrok::Quantity::APPARENT_POWER, Quantity::ApparentPower },
	{ sigrok::Quantity::MASS, Quantity::Mass },
	{ sigrok::Quantity::HARMONIC_RATIO, Quantity::HarmonicRatio },
};

map<Quantity, const sigrok::Quantity *> quantity_sr_quantity_map = {
	{ Quantity::Voltage, sigrok::Quantity::VOLTAGE },
	{ Quantity::Current, sigrok::Quantity::CURRENT },
	{ Quantity::Resistance, sigrok::Quantity::RESISTANCE },
	{ Quantity::Capacitance, sigrok::Quantity::CAPACITANCE },
	{ Quantity::Temperature, sigrok::Quantity::TEMPERATURE },
	{ Quantity::Frequency, sigrok::Quantity::FREQUENCY },
	{ Quantity::DutyCyle, sigrok::Quantity::DUTY_CYCLE },
	{ Quantity::Continuity, sigrok::Quantity::CONTINUITY },
	{ Quantity::PulseWidth, sigrok::Quantity::PULSE_WIDTH },
	{ Quantity::Conductance, sigrok::Quantity::CONDUCTANCE },
	{ Quantity::Power, sigrok::Quantity::POWER },
	{ Quantity::Gain, sigrok::Quantity::GAIN },
	{ Quantity::SoundPressureLevel, sigrok::Quantity::SOUND_PRESSURE_LEVEL },
	{ Quantity::CarbonMonoxide, sigrok::Quantity::CARBON_MONOXIDE },
	{ Quantity::RelativeHumidity, sigrok::Quantity::RELATIVE_HUMIDITY },
	{ Quantity::Time, sigrok::Quantity::TIME },
	{ Quantity::WindSpeed, sigrok::Quantity::WIND_SPEED },
	{ Quantity::Pressure, sigrok::Quantity::PRESSURE },
	{ Quantity::ParallelInductance, sigrok::Quantity::PARALLEL_INDUCTANCE },
	{ Quantity::ParallelCapacitance, sigrok::Quantity::PARALLEL_CAPACITANCE },
	{ Quantity::ParallelResistance, sigrok::Quantity::PARALLEL_RESISTANCE },
	{ Quantity::SeriesInductance, sigrok::Quantity::SERIES_INDUCTANCE },
	{ Quantity::SeriesCapacitance,  sigrok::Quantity::SERIES_CAPACITANCE },
	{ Quantity::SeriesResistance, sigrok::Quantity::SERIES_RESISTANCE },
	{ Quantity::DissipationFactor, sigrok::Quantity::DISSIPATION_FACTOR },
	{ Quantity::QualityFactor, sigrok::Quantity::QUALITY_FACTOR },
	{ Quantity::PhaseAngle, sigrok::Quantity::PHASE_ANGLE },
	{ Quantity::Difference, sigrok::Quantity::DIFFERENCE },
	{ Quantity::Count, sigrok::Quantity::COUNT },
	{ Quantity::PowerFactor, sigrok::Quantity::POWER_FACTOR },
	{ Quantity::ApparentPower, sigrok::Quantity::APPARENT_POWER },
	{ Quantity::Mass, sigrok::Quantity::MASS },
	{ Quantity::HarmonicRatio, sigrok::Quantity::HARMONIC_RATIO },
};

map<const sigrok::QuantityFlag *, QuantityFlag> sr_quantity_flag_quantity_flag_map = {
	{ sigrok::QuantityFlag::AC, QuantityFlag::AC },
	{ sigrok::QuantityFlag::DC, QuantityFlag::DC },
	{ sigrok::QuantityFlag::RMS, QuantityFlag::RMS },
	{ sigrok::QuantityFlag::DIODE, QuantityFlag::Diode },
	{ sigrok::QuantityFlag::HOLD, QuantityFlag::Hold },
	{ sigrok::QuantityFlag::MAX, QuantityFlag::Max },
	{ sigrok::QuantityFlag::MIN, QuantityFlag::Min },
	{ sigrok::QuantityFlag::AUTORANGE, QuantityFlag::Autorange },
	{ sigrok::QuantityFlag::RELATIVE, QuantityFlag::Relative },
	{ sigrok::QuantityFlag::SPL_FREQ_WEIGHT_A, QuantityFlag::SplFreqWeightA },
	{ sigrok::QuantityFlag::SPL_FREQ_WEIGHT_C, QuantityFlag::SplFreqWeightC },
	{ sigrok::QuantityFlag::SPL_FREQ_WEIGHT_Z, QuantityFlag::SplFreqWeightZ },
	{ sigrok::QuantityFlag::SPL_FREQ_WEIGHT_FLAT, QuantityFlag::SplFreqWeightFlat },
	{ sigrok::QuantityFlag::SPL_TIME_WEIGHT_S, QuantityFlag::SplTimeWeightS },
	{ sigrok::QuantityFlag::SPL_TIME_WEIGHT_F, QuantityFlag::SplTimeWeightF },
	{ sigrok::QuantityFlag::SPL_LAT, QuantityFlag::SplLAT },
	{ sigrok::QuantityFlag::SPL_PCT_OVER_ALARM, QuantityFlag::SplPctOverAlarm },
	{ sigrok::QuantityFlag::DURATION, QuantityFlag::Duration },
	{ sigrok::QuantityFlag::AVG, QuantityFlag::Avg },
	{ sigrok::QuantityFlag::REFERENCE, QuantityFlag::Reference },
	{ sigrok::QuantityFlag::UNSTABLE, QuantityFlag::Unstable },
	{ sigrok::QuantityFlag::FOUR_WIRE, QuantityFlag::FourWire },
};

map<QuantityFlag, const sigrok::QuantityFlag *> quantity_flag_sr_quantity_flag_map = {
	{ QuantityFlag::AC, sigrok::QuantityFlag::AC },
	{ QuantityFlag::DC, sigrok::QuantityFlag::DC },
	{ QuantityFlag::RMS, sigrok::QuantityFlag::RMS },
	{ QuantityFlag::Diode, sigrok::QuantityFlag::DIODE },
	{ QuantityFlag::Hold, sigrok::QuantityFlag::HOLD },
	{ QuantityFlag::Max, sigrok::QuantityFlag::MAX },
	{ QuantityFlag::Min, sigrok::QuantityFlag::MIN },
	{ QuantityFlag::Autorange, sigrok::QuantityFlag::AUTORANGE },
	{ QuantityFlag::Relative, sigrok::QuantityFlag::RELATIVE },
	{ QuantityFlag::SplFreqWeightA, sigrok::QuantityFlag::SPL_FREQ_WEIGHT_A },
	{ QuantityFlag::SplFreqWeightC, sigrok::QuantityFlag::SPL_FREQ_WEIGHT_C },
	{ QuantityFlag::SplFreqWeightZ, sigrok::QuantityFlag::SPL_FREQ_WEIGHT_Z },
	{ QuantityFlag::SplFreqWeightFlat, sigrok::QuantityFlag::SPL_FREQ_WEIGHT_FLAT },
	{ QuantityFlag::SplTimeWeightS, sigrok::QuantityFlag::SPL_TIME_WEIGHT_S },
	{ QuantityFlag::SplTimeWeightF, sigrok::QuantityFlag::SPL_TIME_WEIGHT_F },
	{ QuantityFlag::SplLAT, sigrok::QuantityFlag::SPL_LAT },
	{ QuantityFlag::SplPctOverAlarm, sigrok::QuantityFlag::SPL_PCT_OVER_ALARM },
	{ QuantityFlag::Duration, sigrok::QuantityFlag::DURATION },
	{ QuantityFlag::Avg, sigrok::QuantityFlag::AVG },
	{ QuantityFlag::Reference, sigrok::QuantityFlag::REFERENCE },
	{ QuantityFlag::Unstable, sigrok::QuantityFlag::UNSTABLE },
	{ QuantityFlag::FourWire, sigrok::QuantityFlag::FOUR_WIRE },
};

map<const sigrok::Unit *, Unit> sr_unit_unit_map = {
	{ sigrok::Unit::VOLT, Unit::Volt },
	{ sigrok::Unit::AMPERE, Unit::Ampere },
	{ sigrok::Unit::OHM, Unit::Ohm },
	{ sigrok::Unit::FARAD, Unit::Farad },
	{ sigrok::Unit::KELVIN, Unit::Kelvin },
	{ sigrok::Unit::CELSIUS, Unit::Celsius },
	{ sigrok::Unit::FAHRENHEIT, Unit::Fahrenheit },
	{ sigrok::Unit::HERTZ, Unit::Hertz },
	{ sigrok::Unit::PERCENTAGE, Unit::Percentage },
	{ sigrok::Unit::BOOLEAN, Unit::Boolean },
	{ sigrok::Unit::SECOND, Unit::Second },
	{ sigrok::Unit::SIEMENS, Unit::Siemens },
	{ sigrok::Unit::DECIBEL_MW, Unit::DecibelMW },
	{ sigrok::Unit::DECIBEL_VOLT, Unit::DecibelVolt },
	{ sigrok::Unit::UNITLESS, Unit::Unitless },
	{ sigrok::Unit::DECIBEL_SPL, Unit::DecibelSpl },
	{ sigrok::Unit::CONCENTRATION, Unit::Concentration },
	{ sigrok::Unit::REVOLUTIONS_PER_MINUTE, Unit::RevolutionsPerMinute },
	{ sigrok::Unit::VOLT_AMPERE, Unit::VoltAmpere },
	{ sigrok::Unit::WATT, Unit::Watt },
	{ sigrok::Unit::WATT_HOUR, Unit::WattHour },
	{ sigrok::Unit::METER_SECOND, Unit::MeterPerSecond },
	{ sigrok::Unit::HECTOPASCAL, Unit::HectoPascal },
	{ sigrok::Unit::HUMIDITY_293K, Unit::Humidity293K },
	{ sigrok::Unit::DEGREE, Unit::Degree },
	{ sigrok::Unit::HENRY, Unit::Henry },
	{ sigrok::Unit::GRAM, Unit::Gram },
	{ sigrok::Unit::CARAT, Unit::Carat },
	{ sigrok::Unit::OUNCE, Unit::Ounce },
	{ sigrok::Unit::TROY_OUNCE, Unit::TroyOunce },
	{ sigrok::Unit::POUND, Unit::Pound },
	{ sigrok::Unit::PENNYWEIGHT, Unit::Pennyweight },
	{ sigrok::Unit::GRAIN, Unit::Grain },
	{ sigrok::Unit::TAEL, Unit::Tael },
	{ sigrok::Unit::MOMME, Unit::Momme },
	{ sigrok::Unit::TOLA, Unit::Tola },
	{ sigrok::Unit::PIECE, Unit::Piece },
};

map<Unit, const sigrok::Unit *> unit_sr_unit_map = {
	{ Unit::Volt, sigrok::Unit::VOLT },
	{ Unit::Ampere, sigrok::Unit::AMPERE },
	{ Unit::Ohm, sigrok::Unit::OHM },
	{ Unit::Farad, sigrok::Unit::FARAD },
	{ Unit::Kelvin, sigrok::Unit::KELVIN },
	{ Unit::Celsius, sigrok::Unit::CELSIUS },
	{ Unit::Fahrenheit, sigrok::Unit::FAHRENHEIT },
	{ Unit::Hertz, sigrok::Unit::HERTZ },
	{ Unit::Percentage, sigrok::Unit::PERCENTAGE },
	{ Unit::Boolean, sigrok::Unit::BOOLEAN },
	{ Unit::Second, sigrok::Unit::SECOND },
	{ Unit::Siemens, sigrok::Unit::SIEMENS },
	{ Unit::DecibelMW, sigrok::Unit::DECIBEL_MW },
	{ Unit::DecibelVolt, sigrok::Unit::DECIBEL_VOLT },
	{ Unit::Unitless, sigrok::Unit::UNITLESS },
	{ Unit::DecibelSpl, sigrok::Unit::DECIBEL_SPL },
	{ Unit::Concentration, sigrok::Unit::CONCENTRATION },
	{ Unit::RevolutionsPerMinute, sigrok::Unit::REVOLUTIONS_PER_MINUTE },
	{ Unit::VoltAmpere, sigrok::Unit::VOLT_AMPERE },
	{ Unit::Watt, sigrok::Unit::WATT },
	{ Unit::WattHour, sigrok::Unit::WATT_HOUR },
	{ Unit::MeterPerSecond, sigrok::Unit::METER_SECOND },
	{ Unit::HectoPascal, sigrok::Unit::HECTOPASCAL },
	{ Unit::Humidity293K, sigrok::Unit::HUMIDITY_293K },
	{ Unit::Degree, sigrok::Unit::DEGREE },
	{ Unit::Henry, sigrok::Unit::HENRY },
	{ Unit::Gram, sigrok::Unit::GRAM },
	{ Unit::Carat, sigrok::Unit::CARAT },
	{ Unit::Ounce, sigrok::Unit::OUNCE },
	{ Unit::TroyOunce, sigrok::Unit::TROY_OUNCE },
	{ Unit::Pound, sigrok::Unit::POUND },
	{ Unit::Pennyweight, sigrok::Unit::PENNYWEIGHT },
	{ Unit::Grain, sigrok::Unit::GRAIN },
	{ Unit::Tael, sigrok::Unit::TAEL },
	{ Unit::Momme, sigrok::Unit::MOMME },
	{ Unit::Tola, sigrok::Unit::TOLA },
	{ Unit::Piece, sigrok::Unit::PIECE },
};

map<const sigrok::DataType *, DataType> sr_data_type_data_type_map = {
	{ sigrok::DataType::UINT64, DataType::UInt64 },
	{ sigrok::DataType::STRING, DataType::String },
	{ sigrok::DataType::BOOL, DataType::Bool },
	{ sigrok::DataType::FLOAT, DataType::Double },
	{ sigrok::DataType::RATIONAL_PERIOD, DataType::RationalPeriod },
	{ sigrok::DataType::RATIONAL_VOLT, DataType::RationalVolt },
	{ sigrok::DataType::KEYVALUE, DataType::KeyValue },
	{ sigrok::DataType::UINT64_RANGE, DataType::Uint64Range },
	{ sigrok::DataType::DOUBLE_RANGE, DataType::DoubleRange },
	{ sigrok::DataType::INT32, DataType::Int32 },
	{ sigrok::DataType::MQ, DataType::MQ },
};

map<DataType, const sigrok::DataType *> data_type_sr_data_type_map = {
	{ DataType::UInt64, sigrok::DataType::UINT64 },
	{ DataType::String, sigrok::DataType::STRING },
	{ DataType::Bool, sigrok::DataType::BOOL },
	{ DataType::Double, sigrok::DataType::FLOAT },
	{ DataType::RationalPeriod, sigrok::DataType::RATIONAL_PERIOD },
	{ DataType::RationalVolt, sigrok::DataType::RATIONAL_VOLT },
	{ DataType::KeyValue, sigrok::DataType::KEYVALUE },
	{ DataType::Uint64Range, sigrok::DataType::UINT64_RANGE },
	{ DataType::DoubleRange, sigrok::DataType::DOUBLE_RANGE },
	{ DataType::Int32, sigrok::DataType::INT32 },
	{ DataType::MQ, sigrok::DataType::MQ },
};

quantity_unit_map_t quantity_unit_map = {
	{ Quantity::Voltage, { Unit::Volt } },
	{ Quantity::Current, { Unit::Ampere } },
	{ Quantity::Resistance, { Unit::Ohm } },
	{ Quantity::Capacitance, { Unit::Farad } },
	{ Quantity::Temperature, { Unit::Kelvin, Unit::Celsius, Unit::Fahrenheit} },
	{ Quantity::Frequency, { Unit::Hertz } },
	{ Quantity::DutyCyle, { Unit::Percentage } },
	{ Quantity::Continuity, { Unit::Ohm, Unit::Boolean } },
	{ Quantity::PulseWidth, { Unit::Percentage } },
	{ Quantity::Conductance, { Unit::Siemens } },
	{ Quantity::Power, { Unit::Watt, Unit::VoltAmpere } },
	{ Quantity::Work, { Unit::WattHour, Unit::Joule } },
	{ Quantity::ElectricCharge, { Unit::AmpereHour, Unit::Coulomb } },
	{ Quantity::Gain, { Unit::Decibel, Unit::Unitless } },
	{ Quantity::SoundPressureLevel, { Unit::DecibelSpl } },
	{ Quantity::CarbonMonoxide, { Unit::Concentration } },
	{ Quantity::RelativeHumidity, { Unit::Humidity293K } },
	{ Quantity::Time, { Unit::Second } },
	{ Quantity::WindSpeed, { Unit::MeterPerSecond } },
	{ Quantity::Pressure, { Unit::HectoPascal } },
	{ Quantity::ParallelInductance, { Unit::Henry } },
	{ Quantity::ParallelCapacitance, { Unit::Farad } },
	{ Quantity::ParallelResistance, { Unit::Ohm } },
	{ Quantity::SeriesInductance, { Unit::Henry } },
	{ Quantity::SeriesCapacitance, { Unit::Farad } },
	{ Quantity::SeriesResistance, { Unit::Ohm } },
	{ Quantity::DissipationFactor, { Unit::Unitless } },
	{ Quantity::QualityFactor, { Unit::Unitless } },
	{ Quantity::PhaseAngle, { Unit::Degree } },
	{ Quantity::Difference, { Unit::Unitless } },
	{ Quantity::Count, { Unit::Piece, Unit::Unitless } },
	{ Quantity::PowerFactor, { Unit::Unitless } },
	{ Quantity::ApparentPower, { Unit::VoltAmpere } },
	{ Quantity::Mass, { Unit::Gram, Unit::Carat, Unit::Ounce, Unit::TroyOunce,
						Unit::Pound, Unit::Pennyweight, Unit::Grain, Unit::Tael,
						Unit::Momme, Unit::Tola} },
	{ Quantity::HarmonicRatio, { Unit::Unitless } },
};

} // namespace

/**
 * Return all known quantities
 *
 * @return The quantity name map
 */
quantity_name_map_t get_quantity_name_map();

/**
 * Return all known quantity flags
 *
 * @return The quantity flags name map
 */
quantity_flag_name_map_t get_quantity_flag_name_map();

/**
 * Return all known units
 *
 * @return The unit name map
 */
unit_name_map_t get_unit_name_map();

/**
 * Return all known data types
 *
 * @return The data type name map
 */
data_type_name_map_t get_data_type_name_map();


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
 * Return the corresponding QunatityFlag for a sigrok QuantityFlag
 *
 * @param sr_quantity_flag The sigrok QuantityFlag
 *
 * @return The QuantityFlag.
 */
QuantityFlag get_quantity_flag(const sigrok::QuantityFlag *sr_quantity_flag);

/**
 * Return the corresponding sigrok QuantityFlag ID for a QuantityFlag
 *
 * @param quantity_flag The QuantityFlag
 *
 * @return The sigrok QuantityFlag ID as uint64_t.
 */
uint64_t get_sr_quantity_flag_id(QuantityFlag quantity_flag);

/**
 * Return the corresponding QunatityFlags as a set for the
 * sigrok QuantityFlags vector
 *
 * @param sr_quantity_flags The sigrok QuantityFlags as vector
 *
 * @return The QuantityFlags as set.
 */
set<QuantityFlag> get_quantity_flags(
	vector<const sigrok::QuantityFlag *> sr_quantity_flags);

/**
 * Return the corresponding QunatityFlags as a set for the
 * sigrok QuantityFlags (uint64_t)
 *
 * @param sr_quantity_flags The sigrok QuantityFlags as uint64_t
 *
 * @return The QuantityFlags as set.
 */
set<QuantityFlag> get_quantity_flags(uint64_t sr_quantity_flags);

/**
 * Return the corresponding QunatityFlags as an uint64_t
 *
 * @param quantity_flags The QuantityFlags as set
 *
 * @return The sigrok QuantityFlags IDs
 */
uint64_t get_sr_quantity_flags_id(set<QuantityFlag> quantity_flags);


/**
 * Return the corresponding Unit for a sigrok Unit
 *
 * @param sr_unit The sigrok Unit
 *
 * @return The Unit.
 */
Unit get_unit(const sigrok::Unit *sr_unit);


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
QString format_quantity_flags(set<QuantityFlag> quantity_flags,
	const QString seperator);

/**
 * Format a measured_quantity_t (pair<Quantity, set<QunatityFlag>>) to a string
 *
 * @param measured_quantity The measured_quantity.
 *
 * @return The formated measured_quantity.
 */
QString format_measured_quantity(measured_quantity_t measured_quantity);

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
QString format_unit(Unit unit, set<QuantityFlag> quantity_flags);

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
