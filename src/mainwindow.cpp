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

#include <memory>

#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "mainwindow.hpp"
#include "src/devicemanager.hpp"
#include "src/session.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/dialogs/connectdialog.hpp"
#include "src/tabs/basetab.hpp"
#include "src/tabs/measurementtab.hpp"
#include "src/tabs/sinktab.hpp"
#include "src/tabs/sourcetab.hpp"

using std::make_shared;

namespace sv
{

MainWindow::MainWindow(DeviceManager &device_manager, QWidget *parent) :
    QMainWindow(parent),
	device_manager_(device_manager)
{
	qRegisterMetaType<util::Timestamp>("util::Timestamp");
	qRegisterMetaType<uint64_t>("uint64_t");

    setup_ui();
	connect_signals();
}

MainWindow::~MainWindow()
{
}

void MainWindow::init_session()
{
	session_ = make_shared<Session>(device_manager_);
}

void MainWindow::init_default_session()
{
	init_session();

	for (auto user_device : device_manager_.user_spec_devices())
		add_tab(user_device);
}

void MainWindow::init_session_with_file(
	string open_file_name, string open_file_format)
{
	init_session();

	open_file_name = open_file_name;
	open_file_format = open_file_format;
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

void MainWindow::remove_session()
{
	// Stop capture while the session still exists so that the UI can be
	// updated in case we're currently running. If so, this will schedule a
	// call to our on_capture_state_changed() slot for the next run of the
	// event loop. We need to have this executed immediately or else it will
	// be dismissed since the session object will be deleted by the time we
	// leave this method and the event loop gets a chance to run again.
	qWarning() << "remove_session(): stop_capture";
	// TODO: for each device?
	//session_->stop_capture();
	QApplication::processEvents();

	// Remove the session from our list of sessions (which also destroys it)
	//sessions_.remove_if([&](shared_ptr<Session> s) {
	//	return s == session_; });
}

shared_ptr<devices::Device> MainWindow::add_tab(
	shared_ptr<devices::HardwareDevice> device)
{
	tabs::TabType type;
	QString title = QString::fromStdString(
		device->sr_hardware_device()->model());

	const auto keys = device->sr_hardware_device()->driver()->config_keys();
	if (keys.count(sigrok::ConfigKey::POWER_SUPPLY))
		type = tabs::TabTypeSource;
	else if (keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD))
		type = tabs::TabTypeSink;
	else
		type = tabs::TabTypeMeasurement;

	QMainWindow *window = new QMainWindow();
	window->setWindowFlags(Qt::Widget);  // Remove Qt::Window flag

	int index = tabWidget->addTab(window, title);
	tabWidget->setCurrentIndex(index);

	window->setDockNestingEnabled(true);

	session_->add_device(device,
		[&](QString message) { session_error("Aquisition failed", message); });

	device_windows_[device] = window;
	last_focused_device_ = device;

	if (type == tabs::TabTypeSource) {
		tabs::SourceTab *sourceTab =
			new tabs::SourceTab(*session_, device, window);
		window->setCentralWidget(sourceTab);
	} else if (type == tabs::TabTypeSink) {
		tabs::SinkTab *sinkTab =
			new tabs::SinkTab(*session_, device, window);
		window->setCentralWidget(sinkTab);
	} else if (type == tabs::TabTypeMeasurement) {
		tabs::MeasurementTab *measurementTab =
			new tabs::MeasurementTab(*session_, device, window);
		window->setCentralWidget(measurementTab);
	} else if (type == tabs::TabTypeViews) {
	}

	return device;
}

void MainWindow::remove_tab(shared_ptr<devices::HardwareDevice> device)
{
	session_->remove_device(device);
}


void MainWindow::setup_ui()
{
	this->resize(724, 444);

	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	setWindowIcon(mainIcon);

	mainToolBar = new QToolBar();

	/* QAction(QObject *parent) must have a parent for Qt < 5.7 */

	actionExit = new QAction(mainToolBar);
	QIcon exitIcon;
	exitIcon.addFile(QStringLiteral(":/icons/application-exit.png"),
		QSize(), QIcon::Normal, QIcon::Off);
	actionExit->setIcon(exitIcon);

	actionAbout = new QAction(mainToolBar);
	QIcon aboutIcon;
	aboutIcon.addFile(QStringLiteral(":/icons/information.svg"),
		QSize(), QIcon::Normal, QIcon::Off);
	actionAbout->setIcon(aboutIcon);

	actionAddTab = new QAction(mainToolBar);
	QIcon tabIcon;
	tabIcon.addFile(QStringLiteral(":/icons/window-new.png"),
		QSize(), QIcon::Normal, QIcon::Off);
	actionAddTab->setIcon(tabIcon);

	mainToolBar->addAction(actionAddTab);
	mainToolBar->addSeparator();
	mainToolBar->addAction(actionAbout);
	mainToolBar->addAction(actionExit);
	this->addToolBar(Qt::TopToolBarArea, mainToolBar);
	statusBar = new QStatusBar();
	this->setStatusBar(statusBar);


	QHBoxLayout *centralLayout = new QHBoxLayout();
	centralLayout->setContentsMargins(2, 2, 2, 2);
	centralWidget = new QWidget();
	centralWidget->setLayout(centralLayout);

	/*
	infoWidget = new QToolBox();
	QPushButton *btn = new QPushButton();
	btn->setText("TEST");
	infoWidget->addItem(btn, "--==TEST==--");
	QLabel *lbl = new QLabel();
	lbl->setText("Hallo 123");
	infoWidget->addItem(lbl, "--==HALLO==--");
	centralLayout->addWidget(infoWidget);
	*/

	tabWidget = new QTabWidget();
	centralLayout->addWidget(tabWidget);

	this->setCentralWidget(centralWidget);

	retranslate_ui();

	tabWidget->setCurrentIndex(1);
}

void MainWindow::connect_signals()
{
	// Actions
	connect(actionAddTab, SIGNAL(triggered(bool)),
		this, SLOT(on_actionAddTab_triggered()));
}

void MainWindow::retranslate_ui()
{
	this->setWindowTitle(QApplication::translate("SmuView", "SmuView", Q_NULLPTR));
	actionExit->setText(QApplication::translate("SmuView", "Exit", Q_NULLPTR));
	actionAbout->setText(QApplication::translate("SmuView", "About", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
	actionAbout->setToolTip(QApplication::translate("SmuView", "About", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
	actionAddTab->setText(QApplication::translate("SmuView", "Add Tab", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
	actionAddTab->setToolTip(QApplication::translate("SmuView", "Add Tab", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
}

void MainWindow::session_error(const QString text, const QString info_text)
{
	QMetaObject::invokeMethod(this, "show_session_error",
		Qt::QueuedConnection, Q_ARG(QString, text),
		Q_ARG(QString, info_text));
}

void MainWindow::show_session_error(const QString text, const QString info_text)
{
	QMessageBox msg(this);
	msg.setText(text);
	msg.setInformativeText(info_text);
	msg.setStandardButtons(QMessageBox::Ok);
	msg.setIcon(QMessageBox::Warning);
	msg.exec();
}

void MainWindow::on_actionAddTab_triggered()
{
	dialogs::ConnectDialog dlg(device_manager_);

	if (dlg.exec())
		add_tab(dlg.get_selected_device());
}

} // namespace sv
