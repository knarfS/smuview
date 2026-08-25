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

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "src/devices/deviceutil.hpp"

using namespace sv::devices::deviceutil;
using sv::devices::ConnectionKey;
using sv::devices::ConfigKey;
using sv::devices::DeviceType;


BOOST_AUTO_TEST_SUITE(DeviceUtilTest)


BOOST_AUTO_TEST_CASE(is_supported_device_test)
{
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::Oscilloscope), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::Multimeter), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::DemoDev), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::SoundLevelMeter), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::Thermometer), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::Hygrometer), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::Energymeter), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::PowerSupply), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::LcrMeter), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::ElectronicLoad), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::Scale), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::SignalGenerator), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::Powermeter), true);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::Multiplexer), true);

	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::LogicAnalyzer), false);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::Demodulator), false);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::UserDevice), false);
	BOOST_CHECK_EQUAL(is_supported_device(DeviceType::Unknown), false);
}

BOOST_AUTO_TEST_CASE(is_source_sink_device_test)
{
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::PowerSupply), true);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::ElectronicLoad), true);

	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::LogicAnalyzer), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::Oscilloscope), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::Multimeter), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::DemoDev), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::SoundLevelMeter), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::Thermometer), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::Hygrometer), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::Energymeter), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::Demodulator), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::LcrMeter), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::Scale), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::SignalGenerator), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::Powermeter), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::Multiplexer), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::UserDevice), false);
	BOOST_CHECK_EQUAL(is_source_sink_device(DeviceType::Unknown), false);
}

BOOST_AUTO_TEST_CASE(is_measurement_device_test)
{
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::Multimeter), true);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::DemoDev), true);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::SoundLevelMeter), true);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::Thermometer), true);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::Hygrometer), true);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::Energymeter), true);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::LcrMeter), true);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::Scale), true);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::SignalGenerator), true);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::Powermeter), true);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::Multiplexer), true);

	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::LogicAnalyzer), false);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::Oscilloscope), false);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::Demodulator), false);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::PowerSupply), false);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::ElectronicLoad), false);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::UserDevice), false);
	BOOST_CHECK_EQUAL(is_measurement_device(DeviceType::Unknown), false);
}

BOOST_AUTO_TEST_CASE(is_demo_device_test)
{
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::DemoDev), true);

	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::LogicAnalyzer), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::Oscilloscope), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::Multimeter), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::SoundLevelMeter), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::Thermometer), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::Hygrometer), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::Energymeter), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::Demodulator), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::PowerSupply), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::LcrMeter), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::ElectronicLoad), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::Scale), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::SignalGenerator), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::Powermeter), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::Multiplexer), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::UserDevice), false);
	BOOST_CHECK_EQUAL(is_demo_device(DeviceType::Unknown), false);
}

BOOST_AUTO_TEST_CASE(is_oscilloscope_device_test)
{
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::Oscilloscope), true);

	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::LogicAnalyzer), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::Multimeter), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::DemoDev), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::SoundLevelMeter), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::Thermometer), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::Hygrometer), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::Energymeter), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::Demodulator), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::PowerSupply), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::LcrMeter), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::ElectronicLoad), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::Scale), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::SignalGenerator), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::Powermeter), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::Multiplexer), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::UserDevice), false);
	BOOST_CHECK_EQUAL(is_oscilloscope_device(DeviceType::Unknown), false);
}

BOOST_AUTO_TEST_CASE(is_user_device_test)
{
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::UserDevice), true);

	BOOST_CHECK_EQUAL(is_user_device(DeviceType::LogicAnalyzer), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::Oscilloscope), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::Multimeter), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::DemoDev), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::SoundLevelMeter), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::Thermometer), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::Hygrometer), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::Energymeter), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::Demodulator), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::PowerSupply), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::LcrMeter), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::ElectronicLoad), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::Scale), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::SignalGenerator), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::Powermeter), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::Multiplexer), false);
	BOOST_CHECK_EQUAL(is_user_device(DeviceType::Unknown), false);
}


BOOST_AUTO_TEST_CASE(get_device_type_by_sr_ck_test)
{
	BOOST_CHECK(get_device_type(sigrok::ConfigKey::OSCILLOSCOPE) == DeviceType::Oscilloscope);
	BOOST_CHECK(get_device_type(sigrok::ConfigKey::AMPLITUDE) == DeviceType::Unknown);
}

BOOST_AUTO_TEST_CASE(get_device_type_by_id_test)
{
	BOOST_CHECK(get_device_type(sigrok::ConfigKey::OSCILLOSCOPE->id()) == DeviceType::Oscilloscope);
	BOOST_CHECK(get_device_type(sigrok::ConfigKey::AMPLITUDE->id()) == DeviceType::Unknown);
}

BOOST_AUTO_TEST_CASE(get_sr_config_key_by_device_type_test)
{
	BOOST_CHECK_EQUAL(get_sr_config_key(DeviceType::Oscilloscope), sigrok::ConfigKey::OSCILLOSCOPE);
	BOOST_CHECK_EQUAL(get_sr_config_key(DeviceType::UserDevice), nullptr);
	BOOST_CHECK_EQUAL(get_sr_config_key(DeviceType::Unknown), nullptr);
}

BOOST_AUTO_TEST_CASE(get_sr_config_key_id_by_device_type_test)
{
	BOOST_CHECK_EQUAL(get_sr_config_key_id(DeviceType::Oscilloscope), sigrok::ConfigKey::OSCILLOSCOPE->id());
	BOOST_CHECK_EQUAL(get_sr_config_key_id(DeviceType::UserDevice), 0);
	BOOST_CHECK_EQUAL(get_sr_config_key_id(DeviceType::Unknown), 0);
}

BOOST_AUTO_TEST_CASE(is_valid_sr_config_key_by_device_type_test)
{
	BOOST_CHECK_EQUAL(is_valid_sr_config_key(DeviceType::Oscilloscope), true);
	BOOST_CHECK_EQUAL(is_valid_sr_config_key(DeviceType::UserDevice), false);
	BOOST_CHECK_EQUAL(is_valid_sr_config_key(DeviceType::Unknown), false);
}


BOOST_AUTO_TEST_CASE(get_connection_key_by_sr_ck_test)
{
	BOOST_CHECK(get_connection_key(sigrok::ConfigKey::CONN) == ConnectionKey::Conn);
	BOOST_CHECK(get_connection_key(sigrok::ConfigKey::AMPLITUDE) == ConnectionKey::Unknown);
}

BOOST_AUTO_TEST_CASE(get_connection_key_by_id_test)
{
	BOOST_CHECK(get_connection_key(sigrok::ConfigKey::CONN->id()) == ConnectionKey::Conn);
	BOOST_CHECK(get_connection_key(sigrok::ConfigKey::AMPLITUDE->id()) == ConnectionKey::Unknown);
}

BOOST_AUTO_TEST_CASE(get_sr_config_key_by_connection_key_test)
{
	BOOST_CHECK_EQUAL(get_sr_config_key(ConnectionKey::Conn), sigrok::ConfigKey::CONN);
	BOOST_CHECK_EQUAL(get_sr_config_key(ConnectionKey::Unknown), nullptr);
}

BOOST_AUTO_TEST_CASE(get_sr_config_key_id_by_connection_key_test)
{
	BOOST_CHECK_EQUAL(get_sr_config_key_id(ConnectionKey::Conn), sigrok::ConfigKey::CONN->id());
	BOOST_CHECK_EQUAL(get_sr_config_key_id(ConnectionKey::Unknown), 0);
}

BOOST_AUTO_TEST_CASE(is_valid_sr_config_key_by_connection_key_test)
{
	BOOST_CHECK_EQUAL(is_valid_sr_config_key(ConnectionKey::Conn), true);
	BOOST_CHECK_EQUAL(is_valid_sr_config_key(ConnectionKey::Unknown), false);
}


BOOST_AUTO_TEST_CASE(get_config_key_by_sr_ck_test)
{
	BOOST_CHECK(get_config_key(sigrok::ConfigKey::AMPLITUDE) == ConfigKey::Amplitude);
	BOOST_CHECK(get_config_key(sigrok::ConfigKey::OSCILLOSCOPE) == ConfigKey::Unknown);
}

BOOST_AUTO_TEST_CASE(get_config_key_by_id_test)
{
	BOOST_CHECK(get_config_key(sigrok::ConfigKey::AMPLITUDE->id()) == ConfigKey::Amplitude);
	BOOST_CHECK(get_config_key(sigrok::ConfigKey::OSCILLOSCOPE->id()) == ConfigKey::Unknown);
}

BOOST_AUTO_TEST_CASE(get_sr_config_key_by_config_key_test)
{
	BOOST_CHECK_EQUAL(get_sr_config_key(ConfigKey::Amplitude), sigrok::ConfigKey::AMPLITUDE);
	BOOST_CHECK_EQUAL(get_sr_config_key(ConfigKey::Unknown), nullptr);
}

BOOST_AUTO_TEST_CASE(get_sr_config_key_id_by_config_key_test)
{
	BOOST_CHECK_EQUAL(get_sr_config_key_id(ConfigKey::Amplitude), sigrok::ConfigKey::AMPLITUDE->id());
	BOOST_CHECK_EQUAL(get_sr_config_key_id(ConfigKey::Unknown), 0);
}

BOOST_AUTO_TEST_CASE(is_valid_sr_config_key_by_config_key_test)
{
	BOOST_CHECK_EQUAL(is_valid_sr_config_key(ConfigKey::Amplitude), true);
	BOOST_CHECK_EQUAL(is_valid_sr_config_key(ConfigKey::Unknown), false);
}


BOOST_AUTO_TEST_CASE(format_device_type_test)
{
	BOOST_CHECK(format_device_type(DeviceType::Oscilloscope) == QString("Oscilloscope"));
	BOOST_CHECK(format_device_type(DeviceType::UserDevice) == QString("Virtual User Device"));
	BOOST_CHECK(format_device_type(DeviceType::Unknown) == QString("Unknown"));
}

BOOST_AUTO_TEST_CASE(format_connection_key_test)
{
	BOOST_CHECK(format_connection_key(ConnectionKey::Conn) == QString("Connection String"));
	BOOST_CHECK(format_connection_key(ConnectionKey::Unknown) == QString("Unknown"));
}

BOOST_AUTO_TEST_CASE(format_config_key_test)
{
	BOOST_CHECK(format_config_key(ConfigKey::Amplitude) == QString("Amplitude"));
	BOOST_CHECK(format_config_key(ConfigKey::Unknown) == QString("Unknown"));
}


BOOST_AUTO_TEST_SUITE_END()
