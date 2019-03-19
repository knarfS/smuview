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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <map>
#include <memory>
#include <string>

#include <QMainWindow>

using std::map;
using std::shared_ptr;
using std::string;

namespace sv {

class DeviceManager;
class Session;

namespace devices {
class HardwareDevice;
}
namespace tabs {
class BaseTab;
}

namespace ui {
namespace views {
class DevicesView;
}
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(DeviceManager &device_manager,
		QWidget *parent = 0);

	~MainWindow();

	void init_session();
	void init_default_session();
	void init_session_with_file(string open_file_name, string open_file_format);
	void save_session();
	void restore_session();

	void add_user_device_tab();
	void add_hw_device_tab(shared_ptr<devices::HardwareDevice> device);
	void remove_tab(string id);

private:
	void setup_ui();
	void connect_signals();
	void session_error(const QString text, const QString info_text);
	void add_tab(QMainWindow *tab_window, QString title, string id);
	void add_welcome_tab();
	void remove_tab(int tab_index);

	DeviceManager &device_manager_;
	shared_ptr<Session> session_;

	map<string, QMainWindow *> tab_window_map_;
	QWidget *central_widget_;
	ui::views::DevicesView *devices_view_;
	QTabWidget *tab_widget_;

private Q_SLOTS:
	void show_session_error(const QString text, const QString info_text);
	void on_tab_close_requested(int);

};

} // namespace sv

#endif // MAINWINDOW_HPP
