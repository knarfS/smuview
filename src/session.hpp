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

#ifndef SESSION_HPP
#define SESSION_HPP

#include <list>
#include <map>
#include <memory>
#include <string>

#include <QObject>
#include <QSettings>

using std::list;
using std::map;
using std::shared_ptr;
using std::string;

namespace sigrok {
class Context;
}

namespace sv {

class DeviceManager;
class MainWindow;

namespace devices {
class BaseDevice;
class HardwareDevice;
class UserDevice;
}

namespace python {
class SmuScriptRunner;
}

class Session : public QObject
{
	Q_OBJECT

public:
	static shared_ptr<sigrok::Context> sr_context;
	// TODO: use std::chrono / std::time
	static double session_start_timestamp;

public:
	explicit Session(DeviceManager &device_manager);
	~Session();

	DeviceManager &device_manager();
	const DeviceManager &device_manager() const;

	map<string, shared_ptr<devices::BaseDevice>> device_map() const;
	list<shared_ptr<devices::HardwareDevice>> connect_device(string conn_string);
	void add_device(shared_ptr<devices::BaseDevice> device);
	shared_ptr<devices::UserDevice> add_user_device();
	void remove_device(shared_ptr<devices::BaseDevice> device);

	shared_ptr<python::SmuScriptRunner> smu_script_runner();
	void run_smu_script(string script_file);

	void set_main_window(MainWindow *main_window);
	MainWindow *main_window() const;

private:
	DeviceManager &device_manager_;
	map<string, shared_ptr<devices::BaseDevice>> device_map_;
	MainWindow *main_window_;
	shared_ptr<python::SmuScriptRunner> smu_script_runner_;

	void free_unused_memory();

private Q_SLOTS:
	void error_handler(const std::string &sender, const std::string &msg);

Q_SIGNALS:
	void device_added(shared_ptr<sv::devices::BaseDevice> device);
	void device_removed(shared_ptr<sv::devices::BaseDevice> device);

};

} // namespace sv

#endif // SESSION_HPP
