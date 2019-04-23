/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include <QDebug>

#include "session.hpp"
#include "config.h"
#include "src/util.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/userdevice.hpp"

using std::function;
using std::make_pair;
using std::make_shared;
using std::map;
using std::shared_ptr;
using std::string;

namespace sigrok {
class Context;
}

namespace sv {

shared_ptr<sigrok::Context> Session::sr_context;
double Session::session_start_timestamp = .0;

Session::Session(DeviceManager &device_manager, MainWindow *main_window) :
	device_manager_(device_manager),
	main_window_(main_window)
{
}

Session::~Session()
{
	for (auto &device : devices_)
		device.second->close();
}

DeviceManager &Session::device_manager()
{
	return device_manager_;
}

const DeviceManager &Session::device_manager() const
{
	return device_manager_;
}

void Session::save_settings(QSettings &settings) const
{
	(QSettings)&settings;

	// TODO: Remove all signal data from settings?
}

void Session::restore_settings(QSettings &settings)
{
	(QSettings)&settings;

	// TODO: Restore all signal data from settings?
}

map<string, shared_ptr<devices::BaseDevice>> Session::devices() const
{
	return devices_;
}

void Session::add_device(shared_ptr<devices::BaseDevice> device,
	function<void (const QString)> error_handler)
{
	assert(device);

	try {
		device->open(error_handler);
	}
	catch (const QString &e) {
		qWarning() << e;
		device.reset();
	}

	devices_.insert(make_pair(device->id(), device));

	Q_EMIT device_added(device);
}

shared_ptr<devices::UserDevice> Session::add_user_device(
	function<void (const QString)> error_handler)
{
	string vendor = "SmuView";
	string model = "User Device";
	string version = SV_VERSION_STRING;

	auto device = make_shared<devices::UserDevice>(
		sr_context, vendor, model, version);
	this->add_device(device, error_handler);

	return device;
}

void Session::remove_device(shared_ptr<devices::BaseDevice> device)
{
	if (device)
		device->close();

	devices_.erase(device->id());

	Q_EMIT device_removed(device);
}

MainWindow *Session::main_window() const
{
	return main_window_;
}

} // namespace sv
