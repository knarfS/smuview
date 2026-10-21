/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <set>
#include <utility>
#include <boost/test/unit_test.hpp>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "src/data/datautil.hpp"
#include "test/testutils/boostutil.hpp"

using std::make_pair;
using std::set;
using namespace sv::data::datautil;
using sv::data::DataType;
using sv::data::Quantity;
using sv::data::QuantityFlag;
using sv::data::Unit;


BOOST_AUTO_TEST_SUITE(DataUtilTest)

BOOST_AUTO_TEST_CASE(all_sr_quantities_round_trip)
{
	for (const auto *sr_q : sigrok::Quantity::values()) {
		BOOST_TEST_CONTEXT(sr_q->name()) {
			const Quantity q = get_quantity(sr_q);
			BOOST_CHECK(q != Quantity::Unknown);
			BOOST_CHECK_EQUAL(get_sr_quantity_id(q), static_cast<uint32_t>(sr_q->id()));
		}
	}
}

BOOST_AUTO_TEST_CASE(quantity_table_matches_enum)
{
	const auto &map = get_quantity_map();
	BOOST_REQUIRE_EQUAL(map.size(), static_cast<size_t>(Quantity::Unknown) + 1);
	for (size_t i = 0; i < map.size(); i++)
		BOOST_CHECK(map[i].quantity == static_cast<Quantity>(i));
}

BOOST_AUTO_TEST_CASE(get_quantity_by_sr_quantity_test)
{
	BOOST_CHECK(get_quantity(sigrok::Quantity::VOLTAGE) == Quantity::Voltage);
	BOOST_CHECK(get_quantity(sigrok::Quantity::ENERGY) == Quantity::Energy);
	BOOST_CHECK(get_quantity(nullptr) == Quantity::Unknown);
}

BOOST_AUTO_TEST_CASE(get_quantity_by_id_test)
{
	BOOST_CHECK(get_quantity(sigrok::Quantity::VOLTAGE->id()) == Quantity::Voltage);
	BOOST_CHECK(get_quantity(sigrok::Quantity::ENERGY->id()) == Quantity::Energy);
	BOOST_CHECK(get_quantity(static_cast<uint32_t>(0)) == Quantity::Unknown);
}

BOOST_AUTO_TEST_CASE(get_sr_quantity_id_by_quantity_test)
{
	BOOST_CHECK_EQUAL(get_sr_quantity_id(Quantity::Voltage), sigrok::Quantity::VOLTAGE->id());
	BOOST_CHECK_EQUAL(get_sr_quantity_id(Quantity::Energy), sigrok::Quantity::ENERGY->id());
	BOOST_CHECK_EQUAL(get_sr_quantity_id(Quantity::Unknown), 0);
}

BOOST_AUTO_TEST_CASE(is_valid_sr_quantity_by_quantity_test)
{
	BOOST_CHECK_EQUAL(is_valid_sr_quantity(Quantity::Voltage), true);
	BOOST_CHECK_EQUAL(is_valid_sr_quantity(Quantity::Energy), true);
	BOOST_CHECK_EQUAL(is_valid_sr_quantity(Quantity::Unknown), false);
}


BOOST_AUTO_TEST_CASE(all_sr_quantitie_flags_round_trip)
{
	for (const auto *sr_qf : sigrok::QuantityFlag::values()) {
		BOOST_TEST_CONTEXT(sr_qf->name()) {
			const QuantityFlag qf = get_quantity_flag(sr_qf);
			BOOST_CHECK(qf != QuantityFlag::Unknown);
			BOOST_CHECK_EQUAL(get_sr_quantity_flag_id(qf),
				static_cast<uint32_t>(sr_qf->id()));
		}
	}
}

BOOST_AUTO_TEST_CASE(quantity_flag_table_matches_enum)
{
	const auto &map = get_quantity_flag_map();
	BOOST_REQUIRE_EQUAL(map.size(), static_cast<size_t>(QuantityFlag::Unknown) + 1);
	for (size_t i = 0; i < map.size(); i++)
		BOOST_CHECK(map[i].quantity_flag == static_cast<QuantityFlag>(i));
}

BOOST_AUTO_TEST_CASE(get_quantity_flag_by_sr_quantity_flag_test)
{
	BOOST_CHECK(get_quantity_flag(sigrok::QuantityFlag::AC) == QuantityFlag::AC);
	BOOST_CHECK(get_quantity_flag(sigrok::QuantityFlag::FOUR_WIRE) == QuantityFlag::FourWire);
	BOOST_CHECK(get_quantity_flag(nullptr) == QuantityFlag::Unknown);
}

BOOST_AUTO_TEST_CASE(get_quantity_flag_by_id_test)
{
	BOOST_CHECK(get_quantity_flag(sigrok::QuantityFlag::AC->id()) == QuantityFlag::AC);
	BOOST_CHECK(get_quantity_flag(sigrok::QuantityFlag::FOUR_WIRE->id()) == QuantityFlag::FourWire);
	BOOST_CHECK(get_quantity_flag(static_cast<uint32_t>(0)) == QuantityFlag::Unknown);
}

BOOST_AUTO_TEST_CASE(get_sr_quantity_flag_id_by_quantity_flag_test)
{
	BOOST_CHECK_EQUAL(get_sr_quantity_flag_id(QuantityFlag::AC), sigrok::QuantityFlag::AC->id());
	BOOST_CHECK_EQUAL(get_sr_quantity_flag_id(QuantityFlag::FourWire), sigrok::QuantityFlag::FOUR_WIRE->id());
	BOOST_CHECK_EQUAL(get_sr_quantity_flag_id(QuantityFlag::Unknown), 0);
}

BOOST_AUTO_TEST_CASE(get_quantity_flags_by_sr_quantity_flags_test)
{
	BOOST_CHECK((
		get_quantity_flags({sigrok::QuantityFlag::AC, sigrok::QuantityFlag::FOUR_WIRE}) ==
		set<QuantityFlag>{QuantityFlag::AC, QuantityFlag::FourWire}));
}

BOOST_AUTO_TEST_CASE(get_quantity_flags_by_uint64_t_test)
{
	BOOST_CHECK((
		get_quantity_flags(sigrok::QuantityFlag::AC->id() | sigrok::QuantityFlag::FOUR_WIRE->id()) ==
		set<QuantityFlag>{QuantityFlag::AC, QuantityFlag::FourWire}));
}

BOOST_AUTO_TEST_CASE(get_quantity_flags_by_uint64_t_unknown_bits_test)
{
	BOOST_CHECK(get_quantity_flags(0).empty());
	// bit 31 is not a libsigrok flag
	BOOST_CHECK((get_quantity_flags(uint64_t{1} << 31) ==
		set<QuantityFlag>{QuantityFlag::Unknown}));
	// upper 32 bits are ignored
	BOOST_CHECK(get_quantity_flags(uint64_t{1} << 40).empty());
	BOOST_CHECK((get_quantity_flags((uint64_t{1} << 40) | sigrok::QuantityFlag::AC->id()) ==
		set<QuantityFlag>{QuantityFlag::AC}));
}

BOOST_AUTO_TEST_CASE(get_sr_quantity_flags_id_by_quantity_flags_test)
{
	const uint64_t expected = static_cast<uint64_t>(
		sigrok::QuantityFlag::AC->id()) | sigrok::QuantityFlag::FOUR_WIRE->id();
	BOOST_CHECK_EQUAL(
		get_sr_quantity_flags_id({QuantityFlag::AC, QuantityFlag::FourWire}), expected);
}

BOOST_AUTO_TEST_CASE(is_valid_sr_quantity_flag_by_quantity_flag_test)
{
	BOOST_CHECK_EQUAL(is_valid_sr_quantity_flag(QuantityFlag::AC), true);
	BOOST_CHECK_EQUAL(is_valid_sr_quantity_flag(QuantityFlag::FourWire), true);
	BOOST_CHECK_EQUAL(is_valid_sr_quantity_flag(QuantityFlag::Unknown), false);
}


BOOST_AUTO_TEST_CASE(all_sr_units_round_trip)
{
	for (const auto *sr_u : sigrok::Unit::values()) {
		BOOST_TEST_CONTEXT(sr_u->name()) {
			const Unit u = get_unit(sr_u);
			BOOST_CHECK(u != Unit::Unknown);
			BOOST_CHECK_EQUAL(get_sr_unit_id(u), static_cast<uint32_t>(sr_u->id()));
		}
	}
}

BOOST_AUTO_TEST_CASE(unit_table_matches_enum)
{
	const auto &map = get_unit_map();
	BOOST_REQUIRE_EQUAL(map.size(), static_cast<size_t>(Unit::Unknown) + 1);
	for (size_t i = 0; i < map.size(); i++)
		BOOST_CHECK(map[i].unit == static_cast<Unit>(i));
}

BOOST_AUTO_TEST_CASE(get_unit_by_sr_unit_test)
{
	BOOST_CHECK(get_unit(sigrok::Unit::VOLT) == Unit::Volt);
	BOOST_CHECK(get_unit(sigrok::Unit::COULOMB) == Unit::Coulomb);
	BOOST_CHECK(get_unit(nullptr) == Unit::Unknown);
}

BOOST_AUTO_TEST_CASE(get_sr_unit_id_by_unit_test)
{
	BOOST_CHECK_EQUAL(get_sr_unit_id(Unit::Volt), sigrok::Unit::VOLT->id());
	BOOST_CHECK_EQUAL(get_sr_unit_id(Unit::Coulomb), sigrok::Unit::COULOMB->id());
	BOOST_CHECK_EQUAL(get_sr_unit_id(Unit::Unknown), 0);
}


BOOST_AUTO_TEST_CASE(all_sr_data_types_round_trip)
{
	for (const auto *sr_dt : sigrok::DataType::values()) {
		BOOST_TEST_CONTEXT(sr_dt->name()) {
			const DataType dt = get_data_type(sr_dt);
			BOOST_CHECK(dt != DataType::Unknown);
			BOOST_CHECK_EQUAL(get_sr_data_type_id(dt),
				static_cast<uint32_t>(sr_dt->id()));
		}
	}
}

BOOST_AUTO_TEST_CASE(data_type_table_matches_enum)
{
	const auto &map = get_data_type_map();
	BOOST_REQUIRE_EQUAL(map.size(), static_cast<size_t>(DataType::Unknown) + 1);
	for (size_t i = 0; i < map.size(); i++)
		BOOST_CHECK(map[i].data_type == static_cast<DataType>(i));
}

BOOST_AUTO_TEST_CASE(get_data_type_by_sr_data_type_test)
{
	BOOST_CHECK(get_data_type(sigrok::DataType::UINT64) == DataType::UInt64);
	BOOST_CHECK(get_data_type(sigrok::DataType::MQ) == DataType::MQ);
	BOOST_CHECK(get_data_type(nullptr) == DataType::Unknown);
}

BOOST_AUTO_TEST_CASE(get_data_type_by_id_test)
{
	BOOST_CHECK(get_data_type(sigrok::DataType::UINT64->id()) == DataType::UInt64);
	BOOST_CHECK(get_data_type(sigrok::DataType::MQ->id()) == DataType::MQ);
	BOOST_CHECK(get_data_type(static_cast<uint32_t>(0)) == DataType::Unknown);
}

BOOST_AUTO_TEST_CASE(get_sr_data_type_by_data_type_test)
{
	BOOST_CHECK(get_sr_data_type(DataType::UInt64) == sigrok::DataType::UINT64);
	BOOST_CHECK(get_sr_data_type(DataType::MQ) == sigrok::DataType::MQ);
	BOOST_CHECK(get_sr_data_type(DataType::Unknown) == nullptr);
}

BOOST_AUTO_TEST_CASE(get_sr_data_type_id_by_data_type_test)
{
	BOOST_CHECK_EQUAL(get_sr_data_type_id(DataType::UInt64), sigrok::DataType::UINT64->id());
	BOOST_CHECK_EQUAL(get_sr_data_type_id(DataType::MQ), sigrok::DataType::MQ->id());
	BOOST_CHECK_EQUAL(get_sr_data_type_id(DataType::Unknown), 0);
}

BOOST_AUTO_TEST_CASE(is_valid_sr_data_type_by_data_type_test)
{
	BOOST_CHECK_EQUAL(is_valid_sr_data_type(DataType::UInt64), true);
	BOOST_CHECK_EQUAL(is_valid_sr_data_type(DataType::MQ), true);
	BOOST_CHECK_EQUAL(is_valid_sr_data_type(DataType::Unknown), false);
}


BOOST_AUTO_TEST_CASE(format_quantity_test)
{
	BOOST_CHECK_EQUAL(format_quantity(Quantity::Voltage), QString("Voltage"));
	BOOST_CHECK_EQUAL(format_quantity(Quantity::Energy), QString("Energy"));
	BOOST_CHECK_EQUAL(format_quantity(Quantity::Unknown), QString("Unknown"));
}

BOOST_AUTO_TEST_CASE(format_quantity_flag_test)
{
	BOOST_CHECK_EQUAL(format_quantity_flag(QuantityFlag::AC), QString("AC"));
	BOOST_CHECK_EQUAL(format_quantity_flag(QuantityFlag::FourWire), QString("4W"));
	BOOST_CHECK_EQUAL(format_quantity_flag(QuantityFlag::Unknown), QString("Unknown"));
}

BOOST_AUTO_TEST_CASE(format_quantity_flags_test)
{
	BOOST_CHECK_EQUAL(
		format_quantity_flags({QuantityFlag::AC, QuantityFlag::Unknown, QuantityFlag::FourWire}, " "),
		QString("AC 4W Unknown"));
	BOOST_CHECK_EQUAL(
		format_quantity_flags({QuantityFlag::FourWire, QuantityFlag::Avg, QuantityFlag::Max,
			QuantityFlag::Min, QuantityFlag::RMS, QuantityFlag::DC, QuantityFlag::AC, QuantityFlag::Diode}, "-"),
		QString("AC-DC-RMS-min-max-avg-Diode-4W"));
}

BOOST_AUTO_TEST_CASE(format_measured_quantity_test)
{
	BOOST_CHECK_EQUAL(
		format_measured_quantity(make_pair(Quantity::Voltage, set<QuantityFlag>{})),
		QString("Voltage"));
	BOOST_CHECK_EQUAL(
		format_measured_quantity(make_pair(Quantity::Energy,
			set<QuantityFlag>{QuantityFlag::AC, QuantityFlag::Avg})),
		QString("Energy AC avg"));
	BOOST_CHECK_EQUAL(
		format_measured_quantity(make_pair(Quantity::Unknown,
			set<QuantityFlag>{QuantityFlag::Unknown})),
		QString("Unknown Unknown"));
}

BOOST_AUTO_TEST_CASE(format_unit_test)
{
	BOOST_CHECK_EQUAL(format_unit(Unit::Volt), QString("V"));
	BOOST_CHECK_EQUAL(format_unit(Unit::Coulomb), QString("C"));
	BOOST_CHECK_EQUAL(format_unit(Unit::Unknown), QString("??"));
}

BOOST_AUTO_TEST_CASE(format_unit_by_unit_and_quantity_flags_test)
{
	BOOST_CHECK_EQUAL(format_unit(Unit::Volt, set<QuantityFlag>{}), QString("V"));
	BOOST_CHECK_EQUAL(format_unit(Unit::Volt, set<QuantityFlag>{QuantityFlag::AC}), QString("V AC"));
	BOOST_CHECK_EQUAL(format_unit(Unit::Ampere, set<QuantityFlag>{QuantityFlag::Avg}), QString("A"));
	BOOST_CHECK_EQUAL(format_unit(Unit::Ampere, set<QuantityFlag>{QuantityFlag::DC, QuantityFlag::Avg}), QString("A DC"));
	BOOST_CHECK_EQUAL(format_unit(Unit::Ampere, set<QuantityFlag>{QuantityFlag::DC, QuantityFlag::AC}), QString("A AC DC"));
	BOOST_CHECK_EQUAL(format_unit(Unit::Coulomb, set<QuantityFlag>{QuantityFlag::AC}), QString("C"));
	BOOST_CHECK_EQUAL(format_unit(Unit::Unknown, set<QuantityFlag>{QuantityFlag::AC}), QString("??"));
}

BOOST_AUTO_TEST_CASE(format_data_type_test)
{
	BOOST_CHECK_EQUAL(format_data_type(DataType::UInt64), QString("UInt64"));
	BOOST_CHECK_EQUAL(format_data_type(DataType::MQ), QString("Measured Quantity"));
	BOOST_CHECK_EQUAL(format_data_type(DataType::Unknown), QString("Unknown"));
}

BOOST_AUTO_TEST_CASE(get_units_from_quantity_test)
{
	BOOST_CHECK((get_units_from_quantity(Quantity::Voltage) == set<Unit>{Unit::Volt}));
	BOOST_CHECK((get_units_from_quantity(Quantity::Energy) == set<Unit>{Unit::WattHour, Unit::Joule}));
	BOOST_CHECK((get_units_from_quantity(Quantity::Unknown) == set<Unit>{}));
}

BOOST_AUTO_TEST_SUITE_END()
