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

#include <map>
#include <memory>
#include <string>
#include <utility>

#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSizePolicy>
#include <QVBoxLayout>

#include "mainwindow.hpp"
#include "config.h"
#include "src/devicemanager.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/devices/measurementdevice.hpp"
#include "src/devices/sourcesinkdevice.hpp"
#include "src/devices/userdevice.hpp"
#include "src/channels/basechannel.hpp"
#include "src/python/smuscriptrunner.hpp"
#include "src/ui/dialogs/connectdialog.hpp"
#include "src/ui/tabs/basetab.hpp"
#include "src/ui/tabs/devicetab.hpp"
#include "src/ui/tabs/smuscripttab.hpp"
#include "src/ui/tabs/tabhelper.hpp"
#include "src/ui/tabs/welcometab.hpp"
#include "src/ui/views/devicesview.hpp"
#include "src/ui/views/smuscriptview.hpp"

using std::make_pair;
using std::make_shared;
using std::map;
using std::shared_ptr;
using std::string;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(std::shared_ptr<sv::devices::BaseDevice>)
Q_DECLARE_METATYPE(std::shared_ptr<sv::channels::BaseChannel>)
Q_DECLARE_METATYPE(std::shared_ptr<sv::data::BaseSignal>)

namespace sv
{

MainWindow::MainWindow(DeviceManager &device_manager, QWidget *parent) :
	QMainWindow(parent),
	device_manager_(device_manager)
{
	qRegisterMetaType<util::Timestamp>("util::Timestamp");
	qRegisterMetaType<uint64_t>("uint64_t");
	qRegisterMetaType<std::string>("std::string");
	qRegisterMetaType<Qt::DockWidgetArea>("Qt::DockWidgetArea");
	qRegisterMetaType<shared_ptr<devices::BaseDevice>>("shared_ptr<sv::devices::BaseDevice>");
	qRegisterMetaType<shared_ptr<devices::Configurable>>("shared_ptr<sv::devices::Configurable>");
	qRegisterMetaType<shared_ptr<devices::HardwareDevice>>("shared_ptr<sv::devices::HardwareDevice>");
	qRegisterMetaType<shared_ptr<channels::BaseChannel>>("shared_ptr<sv::channels::BaseChannel>");
	qRegisterMetaType<shared_ptr<data::BaseSignal>>("shared_ptr<sv::data::BaseSignal>");
	qRegisterMetaType<shared_ptr<data::AnalogTimeSignal>>("shared_ptr<sv::data::AnalogTimeSignal>");
	qRegisterMetaType<devices::ConfigKey>("devices::ConfigKey");

	init_session();
	setup_ui();
	connect_signals();
}

MainWindow::~MainWindow()
{
}

void MainWindow::init_session()
{
	session_ = make_shared<Session>(device_manager_, this);
}

void MainWindow::init_default_session()
{
	if (device_manager_.user_spec_devices().empty()) {
		// Display the WelcomeTab if no DeviceTabs will be opened, because
		// without a tab in the QTabWidget the main window looks so empty...
		add_welcome_tab();
		return;
	}

	for (const auto &device : device_manager_.user_spec_devices()) {
		// NOTE: add_device() must be called, before the device tab
		//       tries to access the device (device is not opend yet).
		session_->add_device(device);
		add_device_tab(device);
	}
}

void MainWindow::init_session_with_file(
	string open_file_name, string open_file_format)
{
	(void)open_file_name;
	(void)open_file_format;
	// TODO
	//session_->load_init_file(open_file_name, open_file_format);
}

void MainWindow::save_session()
{
	QSettings settings;

	settings.beginGroup("Session");
	settings.remove("");  // Remove all keys in this group
	session_->save_settings(settings);
	settings.endGroup();
}

void MainWindow::restore_session()
{
	QSettings settings;

	settings.beginGroup("Session");
	session_->restore_settings(settings);
	settings.endGroup();
}

void MainWindow::run_smu_script(string script_file)
{
	add_smuscript_tab(script_file);
	session_->smu_script_runner()->run(script_file);
}

void MainWindow::add_tab(ui::tabs::BaseTab *tab_window,
	QString title, string tab_id) // TODO
{
	int index = tab_widget_->addTab(tab_window, title);
	tab_widget_->setCurrentIndex(index);

	tab_window_map_.insert(make_pair(tab_id, tab_window));
}

void MainWindow::add_welcome_tab()
{
	auto *tab = new ui::tabs::WelcomeTab(*session_);
	string tab_id = "welcometab"; // TODO
	add_tab(tab, tr("Welcome"), tab_id);
}

void MainWindow::add_smuscript_tab(string file_name)
{
	auto *tab = new ui::tabs::SmuScriptTab(*session_, file_name);
	string tab_id = "smuscripttab" + file_name; // TODO
	add_tab(tab, tr("SmuScript"), tab_id);
}

void MainWindow::remove_tab(string tab_id)
{
	remove_tab(tab_widget_->indexOf(tab_window_map_[tab_id]));
}

void MainWindow::remove_tab(int tab_index)
{
	QWidget *tab_window = tab_widget_->widget(tab_index);

	tab_widget_->removeTab(tab_index);

	for (const auto &pair : tab_window_map_) {
		if (pair.second == tab_window) {
			tab_window_map_.erase(pair.first);
			break;
		}
	}

	//tab_window->deleteLater();
	delete tab_window;

	if (tab_window_map_.empty()) {
		// When there are no more tabs, display the WelcomeTab
		add_welcome_tab();
	}
}

void MainWindow::change_tab_icon(string tab_id, QIcon icon)
{
	int tab_index = tab_widget_->indexOf(tab_window_map_[tab_id]);
	tab_widget_->setTabIcon(tab_index, icon);
}

ui::tabs::BaseTab *MainWindow::get_base_tab_from_device_id(const string tab_id)
{
	if (tab_window_map_.count(tab_id) > 0)
		return tab_window_map_[tab_id];
	else
		return nullptr;
}

void MainWindow::setup_ui()
{
	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(mainIcon);

	QString window_title = QString("%1 %2").
		arg(tr("SmuView")).arg(SV_VERSION_STRING);
	this->setWindowTitle(window_title);

	QHBoxLayout *centralLayout = new QHBoxLayout();
	centralLayout->setContentsMargins(2, 2, 2, 2);
	central_widget_ = new QWidget();
	central_widget_->setLayout(centralLayout);

	// Tab Widget
	tab_widget_ = new QTabWidget();
	tab_widget_->setTabsClosable(true);
	connect(tab_widget_, SIGNAL(tabCloseRequested(int)),
		this, SLOT(on_tab_close_requested(int)));
	centralLayout->addWidget(tab_widget_);

	this->setCentralWidget(central_widget_);

	// DeviceTreeView Dock
	devices_view_ = new ui::views::DevicesView(*session_);
	devices_view_->setSizePolicy(
		QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

	// A layout must be set to the central widget of the main window
	// before dev_dock->setWidget() is called.
	QDockWidget* dev_dock = new QDockWidget(devices_view_->title());
	dev_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	dev_dock->setContextMenuPolicy(Qt::PreventContextMenu);
	dev_dock->setFeatures(QDockWidget::DockWidgetMovable |
		QDockWidget::DockWidgetFloatable);
	dev_dock->setWidget(devices_view_);
	this->addDockWidget(Qt::LeftDockWidgetArea, dev_dock);

	// This fixes a qt bug. See: https://bugreports.qt.io/browse/QTBUG-65592
	// resizeDocks() was introduced in Qt 5.6.
	this->resizeDocks({dev_dock}, {40}, Qt::Horizontal);

	// SmuScript Dock
	smu_script_view_ = new ui::views::SmuScriptView(*session_);

	QDockWidget* script_dock = new QDockWidget(smu_script_view_->title());
	script_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	script_dock->setContextMenuPolicy(Qt::PreventContextMenu);
	script_dock->setFeatures(QDockWidget::DockWidgetMovable |
		QDockWidget::DockWidgetFloatable);
	script_dock->setWidget(smu_script_view_);
	this->tabifyDockWidget(dev_dock, script_dock);

	// Select device tree dock tab
	dev_dock->show();
	dev_dock->raise();
}

void MainWindow::connect_signals()
{
	// Connect error handlers
	connect(session_->smu_script_runner().get(), &python::SmuScriptRunner::script_error,
		this, &MainWindow::error_handler);
}

void MainWindow::error_handler(const std::string sender, const std::string msg)
{
	QMessageBox msg_box(this);
	msg_box.setText(QString::fromStdString(sender));
	msg_box.setInformativeText(QString::fromStdString(msg));
	msg_box.setStandardButtons(QMessageBox::Ok);
	msg_box.setIcon(QMessageBox::Critical);
	msg_box.exec();
}

void MainWindow::on_tab_close_requested(int index)
{
	auto *tab_window = (ui::tabs::BaseTab *)tab_widget_->widget(index);
	if (tab_window->request_close())
		remove_tab(index);
}

void MainWindow::add_device_tab(
	shared_ptr<sv::devices::BaseDevice> device)
{
	auto *tab = ui::tabs::tabhelper::get_tab_for_device(*session_, device);
	add_tab(tab, device->short_name(), device->id());

	// Connect device error handler
	connect(device.get(), &sv::devices::BaseDevice::device_error,
		this, &MainWindow::error_handler);
}

} // namespace sv
