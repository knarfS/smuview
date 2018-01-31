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

#ifndef SESSION_HPP
#define SESSION_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include <QObject>
#include <QSettings>

using std::function;
using std::mutex;
using std::shared_ptr;
using std::string;
using std::unordered_set;
using std::vector;

namespace sigrok {
class Context;
}

namespace sv {

class DeviceManager;

namespace data {
class SignalBase;
}

namespace devices {
class Device;
}

class Session : public QObject
{
    Q_OBJECT

public:
	static shared_ptr<sigrok::Context> sr_context;

public:
	Session(DeviceManager &device_manager);
	~Session();

	DeviceManager& device_manager();
	const DeviceManager& device_manager() const;

	void save_settings(QSettings &settings) const;
	void restore_settings(QSettings &settings);

	unordered_set<shared_ptr<devices::Device>> devices() const;
	void add_device(shared_ptr<devices::Device> device,
		function<void (const QString)> error_handler);
	void remove_device(shared_ptr<devices::Device> device);

	void add_signal(shared_ptr<data::SignalBase> signal);

	void load_init_file(const string &file_name, const string &format);

private:
	DeviceManager &device_manager_;
	unordered_set<shared_ptr<devices::Device>> devices_;
	unordered_set<shared_ptr<data::SignalBase>> all_signals_;

	void free_unused_memory();

};

} // namespace sv

#endif // SESSION_HPP
