/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

namespace sv {
namespace data {

enum class Quantity
{
	VOLTAGE,
	CURRENT,
	RESISTANCE,
	CAPACITANCE,
	TEMPERATURE,
	FREQUENCY,
	DUTY_CYCLE,
	CONTINUITY,
	PULSE_WIDTH,
	CONDUCTANCE,
	POWER, // Electrical power, usually in W, or dBm.
	GAIN, // Gain (a transistor's gain, or hFE, for example).
	SOUND_PRESSURE_LEVEL, // Logarithmic representation of sound pressure relative to a reference value.
	CARBON_MONOXIDE,
	RELATIVE_HUMIDITY,
	TIME,
	WIND_SPEED,
	PRESSURE,
	PARALLEL_INDUCTANCE,
	PARALLEL_CAPACITANCE,
	PARALLEL_RESISTANCE,
	SERIES_INDUCTANCE,
	SERIES_CAPACITANCE,
	SERIES_RESISTANCE,
	DISSIPATION_FACTOR,
	QUALITY_FACTOR,
	PHASE_ANGLE,
	DIFFERENCE, // Difference from reference value.
	COUNT,
	POWER_FACTOR,
	APPARENT_POWER,
	MASS,
	HARMONIC_RATIO
};

enum class QunatityFlag
{
	AC,
	DC,
	RMS,
	// Value is voltage drop across a diode, or NAN.
	DIODE,
	// Device is in "hold" mode (repeating the last measurement).
	HOLD,
	// Device is in "max" mode, only updating upon a new max value.
	MAX,
	// Device is in "min" mode, only updating upon a new min value.
	MIN,
	AUTORANGE,
	RELATIVE,
	// Sound pressure level is A-weighted in the frequency domain, according to IEC 61672:2003.
	SPL_FREQ_WEIGHT_A,
	// Sound pressure level is C-weighted in the frequency domain, according to IEC 61672:2003.
	SPL_FREQ_WEIGHT_C,
	// Sound pressure level is Z-weighted
	SPL_FREQ_WEIGHT_Z,
	// Sound pressure level is not weighted in the frequency domain, albeit without standards-defined low and high frequency limits.
	SPL_FREQ_WEIGHT_FLAT,
	// Sound pressure level measurement is S-weighted (1s) in the time domain.
	SPL_TIME_WEIGHT_S,
	// Sound pressure level measurement is F-weighted (125ms) in the time domain.
	SPL_TIME_WEIGHT_F,
	// Sound pressure level is time-averaged (LAT), also known as Equivalent Continuous A-weighted Sound Level (LEQ).
	SPL_LAT,
	// Sound pressure level represented as a percentage of measurements that were over a preset alarm level.
	SPL_PCT_OVER_ALARM,
	// Time is duration (as opposed to epoch, ...).
	DURATION,
	// Device is in "avg" mode, averaging upon each new value.
	AVG,
	// Reference value shown.
	REFERENCE,
	// Unstable value (hasn't settled yet).
	UNSTABLE,
	FOUR_WIRE
};

enum class Unit
{
	VOLT,
	AMPERE,
	OHM,
	FARAD,
	KELVIN,
	CELSIUS,
	FAHRENHEIT,
	HERTZ,
	PERCENTAGE,
	BOOLEAN,
	SECOND,
	SIEMENS,
	DECIBEL_MW,
	DECIBEL_VOLT,
	UNITLESS,
	DECIBEL_SPL,
	CONCENTRATION,
	REVOLUTIONS_PER_MINUTE,
	VOLT_AMPERE,
	WATT,
	WATT_HOUR,
	METER_SECOND,
	HECTOPASCAL,// TODO
	HUMIDITY_293K,
	DEGREE,
	HENRY,
	GRAM,
	CARAT,
	OUNCE,
	TROY_OUNCE,
	POUND, // TODO: Pound (lb) or Metric Pound (lbm)?
	PENNYWEIGHT,
	GRAIN,
	TAEL,
	MOMME,
	TOLA,
	PIECE
};

} // namespace data
} // namespace sv
