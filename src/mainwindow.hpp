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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <list>
#include <map>
#include <memory>

#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QMainWindow>

#include "views/viewbase.hpp"

using std::list;
using std::map;
using std::shared_ptr;

namespace sv {

class DeviceManager;
class Session;

namespace devices {
class Device;
class HardwareDevice;
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
	void remove_session();

	shared_ptr<devices::Device> add_view(
		shared_ptr<devices::HardwareDevice> device);
	void remove_view(shared_ptr<devices::HardwareDevice> device);

private:
	DeviceManager &device_manager_;

	shared_ptr<Session> session_;

	shared_ptr<devices::Device> last_focused_device_;
	map< shared_ptr<devices::Device>, QMainWindow*> device_windows_;

	QIcon icon_red_;
	QIcon icon_green_;
	QIcon icon_grey_;

    QAction *actionExit;
    QAction *actionAbout;
    QAction *actionRun;
    QAction *actionAddView;
    QWidget *centralWidget;
    QTabWidget *tabWidget;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

	void setup_ui();
	void retranslate_ui();
	void session_error(const QString text, const QString info_text);

private Q_SLOTS:
	void show_session_error(const QString text, const QString info_text);
	void on_capture_state_changed(int);
	void on_actionAddView_triggered();
	void on_actionRun_triggered();
};

} // namespace sv

#endif // MAINWINDOW_HPP
