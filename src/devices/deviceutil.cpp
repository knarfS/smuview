/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <map>
#include <memory>
#include <set>

#include "deviceutil.hpp"
#include "src/data/datautil.hpp"

using std::map;
using std::set;
using std::shared_ptr;

namespace sv {
namespace devices {
namespace deviceutil {

device_type_name_map_t get_device_type_name_map()
{
	return device_type_name_map;
}

connection_key_name_map_t get_connection_key_name_map()
{
	return connection_key_name_map;
}

config_key_name_map_t get_config_key_name_map()
{
	return config_key_name_map;
}


bool is_supported_driver(shared_ptr<sigrok::Driver> sr_driver)
{
	assert(sr_driver);

	return is_source_sink_driver(sr_driver) ||
		is_measurement_driver(sr_driver);
}

bool is_source_sink_driver(shared_ptr<sigrok::Driver> sr_driver)
{
	assert(sr_driver);

	const auto keys = sr_driver->config_keys();
	return keys.count(sigrok::ConfigKey::POWER_SUPPLY)
		| keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD);
}

bool is_measurement_driver(shared_ptr<sigrok::Driver> sr_driver)
{
	assert(sr_driver);

	const auto keys = sr_driver->config_keys();
	return keys.count(sigrok::ConfigKey::MULTIMETER)
		| keys.count(sigrok::ConfigKey::SOUNDLEVELMETER)
		| keys.count(sigrok::ConfigKey::THERMOMETER)
		| keys.count(sigrok::ConfigKey::HYGROMETER)
		| keys.count(sigrok::ConfigKey::ENERGYMETER)
		| keys.count(sigrok::ConfigKey::LCRMETER)
		| keys.count(sigrok::ConfigKey::SCALE)
		| keys.count(sigrok::ConfigKey::POWERMETER)
		| keys.count(sigrok::ConfigKey::DEMO_DEV);
}

DeviceType get_device_type(const sigrok::ConfigKey *sr_config_key)
{
	if (sr_config_key_device_type_map.count(sr_config_key) > 0)
		return sr_config_key_device_type_map[sr_config_key];
	return DeviceType::Unknown;
}

DeviceType get_device_type(uint32_t sr_config_key)
{
	const sigrok::ConfigKey *sr_ck = sigrok::ConfigKey::get(sr_config_key);
	return get_device_type(sr_ck);
}

const sigrok::ConfigKey *get_sr_config_key(DeviceType device_type)
{
	return device_type_sr_config_key_map[device_type];
}

uint32_t get_sr_config_key_id(DeviceType device_type)
{
	if (device_type_sr_config_key_map.count(device_type) > 0)
		return device_type_sr_config_key_map[device_type]->id();
	return 0;
}

bool is_valid_sr_config_key(DeviceType device_type)
{
	if (device_type_sr_config_key_map.count(device_type) > 0)
		return true;
	return false;
}


ConnectionKey get_connection_key(const sigrok::ConfigKey *sr_config_key)
{
	if (sr_config_key_connection_key_map.count(sr_config_key) > 0)
		return sr_config_key_connection_key_map[sr_config_key];
	return ConnectionKey::Unknown;
}

ConnectionKey get_connection_key(uint32_t sr_config_key)
{
	const sigrok::ConfigKey *sr_ck = sigrok::ConfigKey::get(sr_config_key);
	return get_connection_key(sr_ck);
}

const sigrok::ConfigKey *get_sr_config_key(ConnectionKey connection_key)
{
	return connection_key_sr_config_key_map[connection_key];
}

uint32_t get_sr_config_key_id(ConnectionKey connection_key)
{
	if (connection_key_sr_config_key_map.count(connection_key) > 0)
		return connection_key_sr_config_key_map[connection_key]->id();
	return 0;
}

bool is_valid_sr_config_key(ConnectionKey connection_key)
{
	if (connection_key_sr_config_key_map.count(connection_key) > 0)
		return true;
	return false;
}


ConfigKey get_config_key(const sigrok::ConfigKey *sr_config_key)
{
	if (sr_config_key_config_key_map.count(sr_config_key) > 0)
		return sr_config_key_config_key_map[sr_config_key];
	return ConfigKey::Unknown;
}

ConfigKey get_config_key(uint32_t sr_config_key)
{
	const sigrok::ConfigKey *sr_ck = sigrok::ConfigKey::get(sr_config_key);
	return get_config_key(sr_ck);
}

const sigrok::ConfigKey *get_sr_config_key(ConfigKey config_key)
{
	return config_key_sr_config_key_map[config_key];
}

uint32_t get_sr_config_key_id(ConfigKey config_key)
{
	if (config_key_sr_config_key_map.count(config_key) > 0)
		return config_key_sr_config_key_map[config_key]->id();
	return 0;
}

bool is_valid_sr_config_key(ConfigKey config_key)
{
	if (config_key_sr_config_key_map.count(config_key) > 0)
		return true;
	return false;
}


QString format_device_type(DeviceType device_type)
{
	if (device_type_name_map.count(device_type) > 0)
		return device_type_name_map[device_type];
	return device_type_name_map[DeviceType::Unknown];
}

QString format_connection_key(ConnectionKey connection_key)
{
	if (connection_key_name_map.count(connection_key) > 0)
		return connection_key_name_map[connection_key];
	return connection_key_name_map[ConnectionKey::Unknown];
}

QString format_config_key(ConfigKey config_key)
{
	if (config_key_name_map.count(config_key) > 0)
		return config_key_name_map[config_key];
	return config_key_name_map[ConfigKey::Unknown];
}


data::DataType get_data_type_for_config_key(ConfigKey config_key)
{
	const sigrok::ConfigKey *sr_ck = get_sr_config_key(config_key);
	if (!sr_ck)
		return data::DataType::Unknown;

	return data::datautil::get_data_type(sr_ck->data_type());
}

data::Unit get_unit_for_config_key(ConfigKey config_key)
{
	if (config_key_unit_map.count(config_key) > 0)
		return config_key_unit_map[config_key];
	return data::Unit::Unknown;
}

} // namespace deviceutil
} // namespace devices
} // namespace sv
