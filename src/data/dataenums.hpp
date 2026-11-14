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

#ifndef DATA_DATAENUMS_HPP
#define DATA_DATAENUMS_HPP

namespace sv {
namespace data {

/**
 * All available and supported data types used by data from/to devices
 */
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

/**
 * All available and supported data quantities
 */
enum class Quantity
{
	Voltage,
	Current,
	Resistance,
	Capacitance,
	Temperature,
	Frequency,
	DutyCycle,
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

/**
 * All available and supported data quantity flags
 */
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

/**
 * All available and supported data units
 */
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

} // namespace data
} // namespace sv

#endif // DATA_DATAENUMS_HPP
