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

#include <map>
#include <set>

#include "deviceutil.hpp"

using std::map;
using std::set;

namespace sv {
namespace devices {
namespace deviceutil {

device_type_key_name_map_t get_device_type_key_name_map()
{
	return device_type_key_name_map;
}

connection_key_name_map_t get_connection_key_name_map()
{
	return connection_key_name_map;
}

config_key_name_map_t get_config_key_name_map()
{
	return config_key_name_map;
}


DeviceTypeKey get_device_type_key(const sigrok::ConfigKey *sr_config_key)
{
	if (sr_config_key_device_type_key_map.count(sr_config_key) > 0)
		return sr_config_key_device_type_key_map[sr_config_key];
	return DeviceTypeKey::Unknown;
}

DeviceTypeKey get_device_type_key(uint32_t sr_config_key)
{
	const sigrok::ConfigKey *sr_ck = sigrok::ConfigKey::get(sr_config_key);
	return get_device_type_key(sr_ck);
}

const sigrok::ConfigKey *get_sr_config_key(DeviceTypeKey device_type_key)
{
	return device_type_key_sr_config_key_map[device_type_key];
}

uint32_t get_sr_config_key_id(DeviceTypeKey device_type_key)
{
	if (device_type_key_sr_config_key_map.count(device_type_key) > 0)
		return device_type_key_sr_config_key_map[device_type_key]->id();
	return 0;
}

bool is_valid_sr_config_key(DeviceTypeKey device_type_key)
{
	if (device_type_key_sr_config_key_map.count(device_type_key) > 0)
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


QString format_device_type_key(DeviceTypeKey device_type_key)
{
	if (device_type_key_name_map.count(device_type_key) > 0)
		return device_type_key_name_map[device_type_key];
	return device_type_key_name_map[DeviceTypeKey::Unknown];
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

} // namespace deviceutil
} // namespace devices
} // namespace sv
