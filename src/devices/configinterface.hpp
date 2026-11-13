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

#ifndef DEVICES_CONFIGINTERFACE_HPP
#define DEVICES_CONFIGINTERFACE_HPP

#include <memory>
#include <vector>

#include <glib.h>

#include "src/data/datautil.hpp"
#include "src/devices/deviceenums.hpp"

using std::shared_ptr;
using std::vector;

namespace sigrok {
class Configurable;
}

namespace sv {
namespace devices {

class ConfigInterface
{

public:
	explicit ConfigInterface(
		const shared_ptr<sigrok::Configurable> sr_configurable);

	bool has_get_config(devices::ConfigKey config_key) const;

	template<typename T> bool get_config(
		devices::ConfigKey config_key, T &value) const;
	/**
	 * Special handling for Container Variants (especially std::tuple, used for
	 * measured quantity, ranges and rationales).
	 * Tuple types are only supported with version >= 2.52 of glibmm, but we
	 * need to use version 2.42, because of mxe. Maybe there is also a bug in
	 * the tuple support?
	 *
	 * TODO: Remove when glibmm >= 2.52
	 */
	bool get_container_config(
		devices::ConfigKey config_key, Glib::VariantContainerBase &value) const;
	/**
	 * Helper function to map to get_container_config().
	 *
	 * TODO: Remove when glibmm >= 2.52
	 */
	bool get_measured_quantity_config(
		devices::ConfigKey config_key, data::measured_quantity_t &value) const;

	bool has_set_config(devices::ConfigKey config_key) const;

	template<typename T> void set_config(devices::ConfigKey config_key, const T value);
	/**
	 * Special handling for Container Variants (especially std::tuple, used for
	 * measured quantity, ranges and rationales).
	 * Tuple types are only supported with version >= 2.52 of glibmm, but we
	 * need to use version 2.42, because of mxe. Maybe there is also a bug in
	 * the tuple support?
	 *
	 * TODO: Remove when glibmm >= 2.52
	 */
	void set_container_config(devices::ConfigKey config_key,
		const vector<Glib::VariantBase> &childs);
	/**
	 * Helper function to map to set_container_config().
	 *
	 * TODO: Remove when glibmm >= 2.52
	 */
	void set_measured_quantity_config(devices::ConfigKey config_key,
		const data::measured_quantity_t &mq);

	bool has_list_config(devices::ConfigKey config_key) const;

	bool list_config(devices::ConfigKey config_key, Glib::VariantContainerBase &gvar);

private:
	const shared_ptr<sigrok::Configurable> sr_configurable_;

};

} // namespace devices
} // namespace sv

#endif // DEVICES_CONFIGINTERFACE_HPP
