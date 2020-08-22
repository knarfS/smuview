/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DEVICEMANAGER_HPP
#define DEVICEMANAGER_HPP

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

using std::list;
using std::map;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace Glib {
class VariantBase;
}

namespace sigrok {
class ConfigKey;
class Context;
class Driver;
}

namespace sv {

namespace devices {
class BaseDevice;
class HardwareDevice;
}

class DeviceManager
{

public:
	DeviceManager(shared_ptr<sigrok::Context> context,
		const vector<std::string> &drivers, bool do_scan);

	~DeviceManager() = default;

	const shared_ptr<sigrok::Context> &context() const;

	shared_ptr<sigrok::Context> context();

	const list<shared_ptr<devices::HardwareDevice>> &devices() const;
	list<shared_ptr<devices::HardwareDevice>> user_spec_devices() const;

	list<shared_ptr<devices::HardwareDevice>> driver_scan(
		const string &driver_name, const vector<string> &driver_opts);

	list<shared_ptr<devices::HardwareDevice>> driver_scan(
		shared_ptr<sigrok::Driver> sr_driver,
		const map<const sigrok::ConfigKey *, Glib::VariantBase> &drvopts);

	map<string, string> get_device_info(
		const shared_ptr<devices::BaseDevice> device);

	shared_ptr<devices::HardwareDevice> find_device_from_info(
		const map<string, string> &search_info);

private:
	bool compare_devices(shared_ptr<devices::BaseDevice> a,
		shared_ptr<devices::BaseDevice> b);

	static map<const sigrok::ConfigKey *, Glib::VariantBase>
	driver_scan_options(const vector<string> &user_spec,
		set<const sigrok::ConfigKey *> driver_opts);

protected:
	shared_ptr<sigrok::Context> context_;
	list<shared_ptr<devices::HardwareDevice>> devices_;
	list<shared_ptr<devices::HardwareDevice>> user_spec_devices_;

};

} // namespace sv

#endif // DEVICEMANAGER_HPP
