/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <assert.h>
#include <mutex>

#include <QDebug>

#include "session.hpp"
#include "data/basesignal.hpp"
#include "devices/device.hpp"
#include "devices/hardwaredevice.hpp"

using std::lock_guard;
using std::mutex;

namespace sigrok {
class Context;
}

namespace sv {

shared_ptr<sigrok::Context> Session::sr_context;

Session::Session(DeviceManager &device_manager) :
	device_manager_(device_manager)
{
}

Session::~Session()
{
	// TODO: Close open devices (aka sessions)
}

DeviceManager& Session::device_manager()
{
	return device_manager_;
}

const DeviceManager& Session::device_manager() const
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

void Session::add_device(shared_ptr<devices::HardwareDevice> device,
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

	devices_.insert(device);
}

void Session::remove_device(shared_ptr<devices::HardwareDevice> device)
{
	if (device)
		device->close();

	devices_.erase(device);
}

// TODO: signal/slot??
void Session::add_signal(shared_ptr<data::SignalBase> signal)
{
	all_signals_.insert(signal);
}

} // namespace sv
