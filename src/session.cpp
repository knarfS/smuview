/*
 * This file is part of the SmuView project.
 *
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

#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <QDebug>

#include "session.hpp"
#include "config.h"
#include "src/devicemanager.hpp"
#include "src/util.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/devices/userdevice.hpp"
#include "src/python/smuscriptrunner.hpp"

using std::list;
using std::make_pair;
using std::make_shared;
using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sigrok {
class Context;
}

namespace sv {

shared_ptr<sigrok::Context> Session::sr_context;
double Session::session_start_timestamp = .0;

Session::Session(DeviceManager &device_manager) :
	device_manager_(device_manager)
{
	smu_script_runner_ = make_shared<python::SmuScriptRunner>(*this);
	connect(smu_script_runner_.get(), &python::SmuScriptRunner::script_error,
		this, &Session::error_handler);

	// Connect devices
	for (const auto &device : device_manager.user_spec_devices()) {
		this->add_device(device);
	}
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

list<shared_ptr<devices::HardwareDevice>>
	Session::connect_device(string conn_string)
{
	// Determine the driver name and options (in generic format).
	vector<string> driver_opts = sv::util::split_string(conn_string, ":");
	string driver_name = driver_opts.front();
	driver_opts.erase(driver_opts.begin());

	// Scan for the specified driver, passing scan options.
	list<shared_ptr<devices::HardwareDevice>> devices =
		device_manager_.driver_scan(driver_name, driver_opts);

	for (auto &device : devices)
		add_device(device);

	return devices;
}

void Session::add_device(shared_ptr<devices::BaseDevice> device)
{
	assert(device);

	try {
		device->open();
	}
	catch (const QString &e) {
		qCritical() << e;
		device.reset();
	}

	connect(device.get(), &devices::BaseDevice::device_error,
		this, &Session::error_handler);

	devices_.insert(make_pair(device->id(), device));

	Q_EMIT device_added(device);
}

shared_ptr<devices::UserDevice> Session::add_user_device()
{
	string vendor = "SmuView";
	string model = "User Device";
	string version = SV_VERSION_STRING;

	auto device = make_shared<devices::UserDevice>(
		sr_context, vendor, model, version);
	this->add_device(device);

	return device;
}

void Session::remove_device(shared_ptr<devices::BaseDevice> device)
{
	if (device) {
		device->close();

		disconnect(device.get(), &devices::BaseDevice::device_error,
			this, &Session::error_handler);

		devices_.erase(device->id());

		Q_EMIT device_removed(device);
	}
}


shared_ptr<python::SmuScriptRunner> Session::smu_script_runner()
{
	return smu_script_runner_;
}

void Session::run_smu_script(string script_file)
{
	smu_script_runner_->run(script_file);
}

void Session::set_main_window(MainWindow *main_window)
{
	main_window_ = main_window;
}

MainWindow *Session::main_window() const
{
	return main_window_;
}

void Session::error_handler(const std::string &sender, const std::string &msg)
{
	qCritical() << QString::fromStdString(sender) <<
		" error: " << QString::fromStdString(msg);
}

} // namespace sv
