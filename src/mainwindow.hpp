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
class BaseDevice;
class HardwareDevice;
}

namespace ui {
namespace tabs {
class BaseTab;
}
namespace views {
class DevicesView;
class SmuScriptTreeView;
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

	// TODO: Move to Session, when Session init is in main.cpp
	void run_smu_script(string script_file);

	void add_smuscript_tab(string file_name);
	void remove_tab(string tab_id);
	void change_tab_icon(string tab_id, QIcon icon);
	void change_tab_title(string tab_id, QString title);
	ui::tabs::BaseTab *get_base_tab_from_device_id(const string tab_id);

private:
	void setup_ui();
	void connect_signals();
	void add_tab(ui::tabs::BaseTab *tab_window);
	void add_welcome_tab();
	void remove_tab(int tab_index);

	DeviceManager &device_manager_;
	shared_ptr<Session> session_;

	QWidget *central_widget_;
	ui::views::DevicesView *devices_view_;
	ui::views::SmuScriptTreeView *smu_script_tree_view_;
	QTabWidget *tab_widget_;
	/** tab_window_map_ is used to get the index of the tab in the QTabWidget */
	map<string, ui::tabs::BaseTab *> tab_window_map_;

private Q_SLOTS:
	void error_handler(const std::string &sender, const std::string &msg);
	void on_tab_close_requested(int tab_index);

public Q_SLOTS:
	void add_device_tab(shared_ptr<sv::devices::BaseDevice> device);

};

} // namespace sv

#endif // MAINWINDOW_HPP
