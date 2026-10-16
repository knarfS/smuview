/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <set>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "datautil.hpp"

using std::set;

namespace sv {
namespace data {
namespace datautil {

// TODO: Use tr(), QCoreApplication::translate(), QT_TR_NOOP() or
//       QT_TRANSLATE_NOOP() for translation.
//       See: http://doc.qt.io/qt-5/i18n-source-translation.html

static const std::vector<QuantityMapping> quantity_map = {
	{ Quantity::Voltage,             sigrok::Quantity::VOLTAGE,              QString("Voltage"),              { Unit::Volt } },
	{ Quantity::Current,             sigrok::Quantity::CURRENT,              QString("Current"),              { Unit::Ampere } },
	{ Quantity::Resistance,          sigrok::Quantity::RESISTANCE,           QString("Resistance"),           { Unit::Ohm } },
	{ Quantity::Capacitance,         sigrok::Quantity::CAPACITANCE,          QString("Capacitance"),          { Unit::Farad } },
	{ Quantity::Temperature,         sigrok::Quantity::TEMPERATURE,          QString("Temperature"),          { Unit::Kelvin, Unit::Celsius, Unit::Fahrenheit} },
	{ Quantity::Frequency,           sigrok::Quantity::FREQUENCY,            QString("Frequency"),            { Unit::Hertz } },
	{ Quantity::DutyCycle,           sigrok::Quantity::DUTY_CYCLE,           QString("Duty Cycle"),           { Unit::Percentage } },
	{ Quantity::Continuity,          sigrok::Quantity::CONTINUITY,           QString("Continuity"),           { Unit::Ohm, Unit::Boolean } },
	{ Quantity::PulseWidth,          sigrok::Quantity::PULSE_WIDTH,          QString("Pulse Width"),          { Unit::Percentage } },
	{ Quantity::Conductance,         sigrok::Quantity::CONDUCTANCE,          QString("Conductance"),          { Unit::Siemens } },
	{ Quantity::Power,               sigrok::Quantity::POWER,                QString("Power"),                { Unit::Watt, Unit::VoltAmpere } },
	{ Quantity::ElectricCharge,      sigrok::Quantity::ELECTRIC_CHARGE,      QString("Electric Charge"),      { Unit::AmpereHour, Unit::Coulomb } },
	{ Quantity::Gain,                sigrok::Quantity::GAIN,                 QString("Gain"),                 { /* TODO Unit::Decibel,*/ Unit::Unitless } },
	{ Quantity::SoundPressureLevel,  sigrok::Quantity::SOUND_PRESSURE_LEVEL, QString("Sound Pressure Level"), { Unit::DecibelSpl } },
	{ Quantity::CarbonMonoxide,      sigrok::Quantity::CARBON_MONOXIDE,      QString("Carbon Monoxide"),      { Unit::Concentration } },
	{ Quantity::RelativeHumidity,    sigrok::Quantity::RELATIVE_HUMIDITY,    QString("Relative Humidity"),    { Unit::Humidity293K } },
	{ Quantity::Time,                sigrok::Quantity::TIME,                 QString("Time"),                 { Unit::Second } },
	{ Quantity::WindSpeed,           sigrok::Quantity::WIND_SPEED,           QString("Wind Speed"),           { Unit::MeterPerSecond } },
	{ Quantity::Pressure,            sigrok::Quantity::PRESSURE,             QString("Pressure"),             { Unit::HectoPascal } },
	{ Quantity::ParallelInductance,  sigrok::Quantity::PARALLEL_INDUCTANCE,  QString("Parallel Inductance"),  { Unit::Henry } },
	{ Quantity::ParallelCapacitance, sigrok::Quantity::PARALLEL_CAPACITANCE, QString("Parallel Capacitance"), { Unit::Farad } },
	{ Quantity::ParallelResistance,  sigrok::Quantity::PARALLEL_RESISTANCE,  QString("Parallel Resistance"),  { Unit::Ohm } },
	{ Quantity::SeriesInductance,    sigrok::Quantity::SERIES_INDUCTANCE,    QString("Series Inductance"),    { Unit::Henry } },
	{ Quantity::SeriesCapacitance,   sigrok::Quantity::SERIES_CAPACITANCE,   QString("Series Capacitance"),   { Unit::Farad } },
	{ Quantity::SeriesResistance,    sigrok::Quantity::SERIES_RESISTANCE,    QString("Series Resistance"),    { Unit::Ohm } },
	{ Quantity::DissipationFactor,   sigrok::Quantity::DISSIPATION_FACTOR,   QString("Dissipation Factor"),   { Unit::Unitless } },
	{ Quantity::QualityFactor,       sigrok::Quantity::QUALITY_FACTOR,       QString("Quality Factor"),       { Unit::Unitless } },
	{ Quantity::PhaseAngle,          sigrok::Quantity::PHASE_ANGLE,          QString("Phase Angle"),          { Unit::Degree } },
	{ Quantity::Difference,          sigrok::Quantity::DIFFERENCE,           QString("Difference"),           { Unit::Unitless } },
	{ Quantity::Count,               sigrok::Quantity::COUNT,                QString("Count"),                { Unit::Piece, Unit::Unitless } },
	{ Quantity::PowerFactor,         sigrok::Quantity::POWER_FACTOR,         QString("Power Factor"),         { Unit::Unitless } },
	{ Quantity::ApparentPower,       sigrok::Quantity::APPARENT_POWER,       QString("Apparent Power"),       { Unit::VoltAmpere } },
	{ Quantity::Mass,                sigrok::Quantity::MASS,                 QString("Mass"),                 { Unit::Gram, Unit::Carat, Unit::Ounce, Unit::TroyOunce,
                                                                                                                Unit::Pound, Unit::Pennyweight, Unit::Grain, Unit::Tael,
                                                                                                                Unit::Momme, Unit::Tola} },
	{ Quantity::HarmonicRatio,       sigrok::Quantity::HARMONIC_RATIO,       QString("Harmonic Ratio"),       { Unit::Unitless } },
	{ Quantity::Energy,              sigrok::Quantity::ENERGY,               QString("Energy"),               { Unit::WattHour, Unit::Joule } },
	{ Quantity::Unknown,             nullptr,                                QString("Unknown"),              { } },
};

static const std::vector<QuantityFlagMapping> quantity_flag_map = {
	{ QuantityFlag::AC,                sigrok::QuantityFlag::AC,                   QString("AC") },
	{ QuantityFlag::DC,                sigrok::QuantityFlag::DC,                   QString("DC") },
	{ QuantityFlag::RMS,               sigrok::QuantityFlag::RMS,                  QString("RMS") },
	{ QuantityFlag::Diode,             sigrok::QuantityFlag::DIODE,                QString("Diode") },
	{ QuantityFlag::Hold,              sigrok::QuantityFlag::HOLD,                 QString("Hold") },
	{ QuantityFlag::Max,               sigrok::QuantityFlag::MAX,                  QString("max") },
	{ QuantityFlag::Min,               sigrok::QuantityFlag::MIN,                  QString("min") },
	{ QuantityFlag::Autorange,         sigrok::QuantityFlag::AUTORANGE,            QString("Autorange") },
	{ QuantityFlag::Relative,          sigrok::QuantityFlag::RELATIVE,             QString("Relative") },
	{ QuantityFlag::SplFreqWeightA,    sigrok::QuantityFlag::SPL_FREQ_WEIGHT_A,    QString("SPL A-weighted F") },
	{ QuantityFlag::SplFreqWeightC,    sigrok::QuantityFlag::SPL_FREQ_WEIGHT_C,    QString("SPL C-weighted F") },
	{ QuantityFlag::SplFreqWeightZ,    sigrok::QuantityFlag::SPL_FREQ_WEIGHT_Z,    QString("SPL Z-weighted F") },
	{ QuantityFlag::SplFreqWeightFlat, sigrok::QuantityFlag::SPL_FREQ_WEIGHT_FLAT, QString("SPL flat weighted") },
	{ QuantityFlag::SplTimeWeightS,    sigrok::QuantityFlag::SPL_TIME_WEIGHT_S,    QString("SPL S-weighted t") },
	{ QuantityFlag::SplTimeWeightF,    sigrok::QuantityFlag::SPL_TIME_WEIGHT_F,    QString("SPL F-weighted t") },
	{ QuantityFlag::SplLAT,            sigrok::QuantityFlag::SPL_LAT,              QString("SPL LAT") },
	{ QuantityFlag::SplPctOverAlarm,   sigrok::QuantityFlag::SPL_PCT_OVER_ALARM,   QString("SPL Over%") },
	{ QuantityFlag::Duration,          sigrok::QuantityFlag::DURATION,             QString("Duration") },
	{ QuantityFlag::Avg,               sigrok::QuantityFlag::AVG,                  QString("avg") },
	{ QuantityFlag::Reference,         sigrok::QuantityFlag::REFERENCE,            QString("Reference") },
	{ QuantityFlag::Unstable,          sigrok::QuantityFlag::UNSTABLE,             QString("Unstable") },
	{ QuantityFlag::FourWire,          sigrok::QuantityFlag::FOUR_WIRE,            QString("4W") },
	{ QuantityFlag::Unknown,           nullptr,                                    QString("Unknown") },
};

static const std::vector<UnitMapping> unit_map = {
	{ Unit::Volt,                 sigrok::Unit::VOLT,                   QString("V") },
	{ Unit::Ampere,               sigrok::Unit::AMPERE,                 QString("A") },
	{ Unit::Ohm,                  sigrok::Unit::OHM,                    QString::fromUtf8("\u2126") },
	{ Unit::Farad,                sigrok::Unit::FARAD,                  QString("F") },
	{ Unit::Kelvin,               sigrok::Unit::KELVIN,                 QString("K") },
	{ Unit::Celsius,              sigrok::Unit::CELSIUS,                QString("°C") },
	{ Unit::Fahrenheit,           sigrok::Unit::FAHRENHEIT,             QString("°F") },
	{ Unit::Hertz,                sigrok::Unit::HERTZ,                  QString("Hz") },
	{ Unit::Percentage,           sigrok::Unit::PERCENTAGE,             QString("%") },
	{ Unit::Boolean,              sigrok::Unit::BOOLEAN,                QString("bool") },
	{ Unit::Second,               sigrok::Unit::SECOND,                 QString("s") },
	{ Unit::Siemens,              sigrok::Unit::SIEMENS,                QString("S") },
	{ Unit::DecibelMW,            sigrok::Unit::DECIBEL_MW,             QString("dBm") },
	{ Unit::DecibelVolt,          sigrok::Unit::DECIBEL_VOLT,           QString("dBV") },
	{ Unit::Unitless,             sigrok::Unit::UNITLESS,               QString("") },
	{ Unit::DecibelSpl,           sigrok::Unit::DECIBEL_SPL,            QString("dB") },
	{ Unit::Concentration,        sigrok::Unit::CONCENTRATION,          QString("ppx") },
	{ Unit::RevolutionsPerMinute, sigrok::Unit::REVOLUTIONS_PER_MINUTE, QString("RPM") },
	{ Unit::VoltAmpere,           sigrok::Unit::VOLT_AMPERE,            QString("VA") },
	{ Unit::Watt,                 sigrok::Unit::WATT,                   QString("W") },
	{ Unit::WattHour,             sigrok::Unit::WATT_HOUR,              QString("Wh") },
	{ Unit::Joule,                sigrok::Unit::JOULE,                  QString("J") },
	{ Unit::AmpereHour,           sigrok::Unit::AMPERE_HOUR,            QString("Ah") },
	{ Unit::Coulomb,              sigrok::Unit::COULOMB,                QString("C") },
	{ Unit::MeterPerSecond,       sigrok::Unit::METER_SECOND,           QString("m/s") },
	{ Unit::HectoPascal,          sigrok::Unit::HECTOPASCAL,            QString("hPa") },
	{ Unit::Humidity293K,         sigrok::Unit::HUMIDITY_293K,          QString("%") },
	{ Unit::Degree,               sigrok::Unit::DEGREE,                 QString("°") },
	{ Unit::Henry,                sigrok::Unit::HENRY,                  QString("H") },
	{ Unit::Gram,                 sigrok::Unit::GRAM,                   QString("g") },
	{ Unit::Carat,                sigrok::Unit::CARAT,                  QString("ct") },
	{ Unit::Ounce,                sigrok::Unit::OUNCE,                  QString("oz.") },
	{ Unit::TroyOunce,            sigrok::Unit::TROY_OUNCE,             QString("oz.tr.") },
	{ Unit::Pound,                sigrok::Unit::POUND,                  QString("lb") },
	{ Unit::Pennyweight,          sigrok::Unit::PENNYWEIGHT,            QString("dwt.") },
	{ Unit::Grain,                sigrok::Unit::GRAIN,                  QString("gr.") },
	{ Unit::Tael,                 sigrok::Unit::TAEL,                   QString::fromUtf8("\u4E24") },
	{ Unit::Momme,                sigrok::Unit::MOMME,                  QString::fromUtf8("\u5301") },
	{ Unit::Tola,                 sigrok::Unit::TOLA,                   QString("tola") },
	{ Unit::Piece,                sigrok::Unit::PIECE,                  QString("pc.") },
	// TODO: Implement in libsigrok
	//{ Unit::Decibel,                                                  QString("dB") },
	{ Unit::Unknown,              nullptr,                              QString("??") },
};

static const std::vector<DataTypeMapping> data_type_map = {
	{ DataType::UInt64,         sigrok::DataType::UINT64,          QString("UInt64") },
	{ DataType::String,         sigrok::DataType::STRING,          QString("String") },
	{ DataType::Bool,           sigrok::DataType::BOOL,            QString("Boolean") },
	{ DataType::Double,         sigrok::DataType::FLOAT,           QString("Double") },
	{ DataType::RationalPeriod, sigrok::DataType::RATIONAL_PERIOD, QString("Rational Period") },
	{ DataType::RationalVolt,   sigrok::DataType::RATIONAL_VOLT,   QString("Rational Volt") },
	{ DataType::KeyValue,       sigrok::DataType::KEYVALUE,        QString("Key Value") },
	{ DataType::UInt64Range,    sigrok::DataType::UINT64_RANGE,    QString("UInt64 Range") },
	{ DataType::DoubleRange,    sigrok::DataType::DOUBLE_RANGE,    QString("Double Range") },
	{ DataType::Int32,          sigrok::DataType::INT32,           QString("Int32") },
	{ DataType::MQ,             sigrok::DataType::MQ,              QString("Measured Quantity") },
	{ DataType::UInt32,         sigrok::DataType::UINT32,          QString("UInt32") },
	{ DataType::Unknown,        nullptr,                           QString("Unknown") },
};


const vector<QuantityMapping> &get_quantity_map()
{
	return quantity_map;
}

const vector<QuantityFlagMapping> &get_quantity_flag_map()
{
	return quantity_flag_map;
}

const vector<UnitMapping> &get_unit_map()
{
	return unit_map;
}

const vector<DataTypeMapping> &get_data_type_map()
{
	return data_type_map;
}


Quantity get_quantity(const sigrok::Quantity *sr_quantity)
{
	if (sr_quantity == nullptr)
		return Quantity::Unknown;

	for (const auto &entry : quantity_map)
		if (entry.sr_quantity == sr_quantity)
			return entry.quantity;
	return Quantity::Unknown;
}

Quantity get_quantity(uint32_t sr_quantity)
{
	try {
		const sigrok::Quantity *sr_q = sigrok::Quantity::get(
			static_cast<int>(sr_quantity));
		return get_quantity(sr_q);
	}
	catch (sigrok::Error const&) {
		return Quantity::Unknown;
	}
}

uint32_t get_sr_quantity_id(Quantity quantity)
{
	for (const auto &entry : quantity_map) {
		if (entry.quantity == quantity) {
			if (entry.sr_quantity == nullptr)
				return 0;
			else
				return entry.sr_quantity->id();
		}
	}
	return 0;
}


QuantityFlag get_quantity_flag(const sigrok::QuantityFlag *sr_quantity_flag)
{
	if (sr_quantity_flag == nullptr)
		return QuantityFlag::Unknown;

	for (const auto &entry : quantity_flag_map)
		if (entry.sr_quantity_flag == sr_quantity_flag)
			return entry.quantity_flag;
	return QuantityFlag::Unknown;
}

QuantityFlag get_quantity_flag(uint32_t sr_quantity_flag)
{
	try {
		const sigrok::QuantityFlag *sr_qf = sigrok::QuantityFlag::get(
			static_cast<int>(sr_quantity_flag));
		return get_quantity_flag(sr_qf);
	}
	catch (sigrok::Error const&) {
		return QuantityFlag::Unknown;
	}
}

uint64_t get_sr_quantity_flag_id(QuantityFlag quantity_flag)
{
	for (const auto &entry : quantity_flag_map) {
		if (entry.quantity_flag == quantity_flag) {
			if (entry.sr_quantity_flag == nullptr)
				return 0;
			else
				return entry.sr_quantity_flag->id();
		}
	}
	return 0;
}

bool is_valid_sr_quantity(data::Quantity quantity)
{
	for (const auto &entry : quantity_map)
		if (entry.quantity == quantity)
			return entry.sr_quantity != nullptr;
	return false;
}

set<QuantityFlag> get_quantity_flags(
	const vector<const sigrok::QuantityFlag *> &sr_quantity_flags)
{
	set<data::QuantityFlag> quantity_flag_set;
	for (const auto &sr_qf : sr_quantity_flags) {
		quantity_flag_set.insert(get_quantity_flag(sr_qf));
	}
	return quantity_flag_set;
}

set<QuantityFlag> get_quantity_flags(uint64_t sr_quantity_flags)
{
	set<data::QuantityFlag> quantity_flag_set;
	uint64_t mask = 1;
	// Silently ignore upper 32 bits. libsigrok can only handle 31 flags,
	// because they are defined in `enum sr_mqflags` which is an `int`
	for (unsigned int i = 0; i < 32; i++, mask <<= 1) {
		if (!(sr_quantity_flags & mask))
			continue;
		quantity_flag_set.insert(get_quantity_flag(
			static_cast<uint32_t>(sr_quantity_flags & mask)));
	}
	return quantity_flag_set;
}

uint64_t get_sr_quantity_flags_id(const set<QuantityFlag> &quantity_flags)
{
	uint64_t sr_qfs_id = 0;
	for (const auto &quantity_flag : quantity_flags) {
		sr_qfs_id |= get_sr_quantity_flag_id(quantity_flag);
	}
	return sr_qfs_id;
}

bool is_valid_sr_quantity_flag(data::QuantityFlag quantity_flag)
{
	for (const auto &entry : quantity_flag_map)
		if (entry.quantity_flag == quantity_flag)
			return entry.sr_quantity_flag != nullptr;
	return false;
}


Unit get_unit(const sigrok::Unit *sr_unit)
{
	if (sr_unit == nullptr)
		return Unit::Unknown;

	for (const auto &entry : unit_map)
		if (entry.sr_unit == sr_unit)
			return entry.unit;
	return Unit::Unknown;
}

uint32_t get_sr_unit_id(Unit unit)
{
	for (const auto &entry : unit_map) {
		if (entry.unit == unit) {
			if (entry.sr_unit == nullptr)
				return 0;
			else
				return entry.sr_unit->id();
		}
	}
	return 0;
}


DataType get_data_type(const sigrok::DataType *sr_data_type)
{
	if (sr_data_type == nullptr)
		return DataType::Unknown;

	for (const auto &entry : data_type_map)
		if (entry.sr_data_type == sr_data_type)
			return entry.data_type;
	return DataType::Unknown;
}

DataType get_data_type(uint32_t sr_data_type)
{
	try {
		const sigrok::DataType *sr_dt = sigrok::DataType::get(
			static_cast<int>(sr_data_type));
		return get_data_type(sr_dt);
	}
	catch (sigrok::Error const&) {
		return DataType::Unknown;
	}
}

const sigrok::DataType *get_sr_data_type(DataType data_type)
{
	for (const auto &entry : data_type_map)
		if (entry.data_type == data_type)
			return entry.sr_data_type;
	return nullptr;
}

uint32_t get_sr_data_type_id(DataType data_type)
{
	const sigrok::DataType *sr_data_type = get_sr_data_type(data_type);
	if (sr_data_type != nullptr)
		return sr_data_type->id();
	return 0;
}

bool is_valid_sr_data_type(DataType data_type)
{
	for (const auto &entry : data_type_map)
		if (entry.data_type == data_type)
			return entry.sr_data_type != nullptr;
	return false;
}


QString format_quantity(Quantity quantity)
{
	for (const auto &entry : quantity_map)
		if (entry.quantity == quantity)
			return entry.name;
	return format_quantity(Quantity::Unknown);
}

QString format_quantity_flag(QuantityFlag quantity_flag)
{
	for (const auto &entry : quantity_flag_map)
		if (entry.quantity_flag == quantity_flag)
			return entry.name;
	return format_quantity_flag(QuantityFlag::Unknown);
}

QString format_quantity_flags(const set<QuantityFlag> &quantity_flags,
	const QString &seperator)
{
	QString qfs_str("");
	QString sep("");

	// Show AC/DC first
	if (quantity_flags.count(QuantityFlag::AC) > 0) {
		qfs_str.append(format_quantity_flag(QuantityFlag::AC));
		sep = seperator;
	}
	if (quantity_flags.count(QuantityFlag::DC) > 0) {
		qfs_str.append(sep);
		qfs_str.append(format_quantity_flag(QuantityFlag::DC));
		sep = seperator;
	}
	// 2nd is RMS
	if (quantity_flags.count(QuantityFlag::RMS) > 0) {
		qfs_str.append(sep);
		qfs_str.append(format_quantity_flag(QuantityFlag::RMS));
		sep = seperator;
	}
	// 3rd is min/max/avg
	if (quantity_flags.count(QuantityFlag::Min) > 0) {
		qfs_str.append(sep);
		qfs_str.append(format_quantity_flag(QuantityFlag::Min));
		sep = seperator;
	}
	if (quantity_flags.count(QuantityFlag::Max) > 0) {
		qfs_str.append(sep);
		qfs_str.append(format_quantity_flag(QuantityFlag::Max));
		sep = seperator;
	}
	if (quantity_flags.count(QuantityFlag::Avg) > 0) {
		qfs_str.append(sep);
		qfs_str.append(format_quantity_flag(QuantityFlag::Avg));
		sep = seperator;
	}

	// And now the rest of the flags
	for (const auto &qf : quantity_flags) {
		if (qf == QuantityFlag::AC || qf == QuantityFlag::DC ||
				qf == QuantityFlag::RMS || qf == QuantityFlag::Min ||
				qf == QuantityFlag::Max || qf == QuantityFlag::Avg)
			continue;

		qfs_str.append(sep);
		qfs_str.append(format_quantity_flag(qf));
		sep = seperator;
	}

	return qfs_str;
}

QString format_measured_quantity(const measured_quantity_t &measured_quantity)
{
	Quantity q = measured_quantity.first; // NOLINT(readability-identifier-length)
	set<QuantityFlag> qfs = measured_quantity.second;
	QString q_qfs_str = format_quantity(q);
	if (!qfs.empty())
		q_qfs_str.append(" ").append(format_quantity_flags(qfs, " "));

	return q_qfs_str;
}

QString format_unit(Unit unit)
{
	for (const auto &entry : unit_map)
		if (entry.unit == unit)
			return entry.name;
	return format_unit(Unit::Unknown);
}

QString format_unit(Unit unit, const set<QuantityFlag> &quantity_flags)
{
	QString unit_str = format_unit(unit);
	if (unit == Unit::Volt || unit == Unit::Ampere) {
		if (quantity_flags.count(QuantityFlag::AC) > 0) {
			unit_str = unit_str.append(" ").append(
				format_quantity_flag(QuantityFlag::AC));
		}
		if (quantity_flags.count(QuantityFlag::DC) > 0) {
			unit_str = unit_str.append(" ").append(
				format_quantity_flag(QuantityFlag::DC));
		}
	}
	return unit_str;
}

QString format_data_type(DataType data_type)
{
	for (const auto &entry : data_type_map)
		if (entry.data_type == data_type)
			return entry.name;
	return format_data_type(DataType::Unknown);
}


set<data::Unit> get_units_from_quantity(data::Quantity quantity)
{
	for (const auto &entry : quantity_map)
		if (entry.quantity == quantity)
			return entry.units;
	return {};
}

} // namespace datautil
} // namespace data
} // namespace sv
