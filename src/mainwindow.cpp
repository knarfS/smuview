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
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "mainwindow.hpp"
#include "devicemanager.hpp"
#include "session.hpp"
#include "data/analogsegment.hpp"
#include "devices/hardwaredevice.hpp"
#include "dialogs/connect.hpp"
#include "src/views/viewbase.hpp"
#include "src/views/sinkview.hpp"
#include "src/views/sourceview.hpp"

using std::make_shared;

Q_DECLARE_METATYPE(shared_ptr<sv::data::AnalogSegment>);

namespace sv
{

MainWindow::MainWindow(DeviceManager &device_manager, QWidget *parent) :
    QMainWindow(parent),
	device_manager_(device_manager),
	icon_red_(":/icons/status-red.svg"),
	icon_green_(":/icons/status-green.svg"),
	icon_grey_(":/icons/status-grey.svg")
{
	qRegisterMetaType<util::Timestamp>("util::Timestamp");
	qRegisterMetaType<uint64_t>("uint64_t");
	qRegisterMetaType<shared_ptr<sv::data::AnalogSegment> >("std::shared_ptr<sv::data::AnalogSegment>");

    setupUi();

	// Actions
	connect(actionAddView, SIGNAL(triggered(bool)),
		this, SLOT(on_actionAddView_triggered()));
	connect(actionRun, SIGNAL(triggered(bool)),
		this, SLOT(on_actionRun_triggered()));
}

MainWindow::~MainWindow()
{
}

void MainWindow::init_session()
{
	shared_ptr<Session> session = make_shared<Session>(device_manager_);

	/*
	connect(session.get(), SIGNAL(add_view(const QString&, views::ViewType, Session*)),
		this, SLOT(on_add_view(const QString&, views::ViewType, Session*)));
	connect(session.get(), SIGNAL(name_changed()),
		this, SLOT(on_session_name_changed()));
	*/

	connect(session.get(), SIGNAL(capture_state_changed(int)),
		this, SLOT(on_capture_state_changed(int)));

	session_ = session;
}

void MainWindow::init_default_session()
{
	init_session();

	for (auto user_device : device_manager_.user_spec_devices())
		add_view(user_device);
}

void MainWindow::init_session_with_file(string open_file_name, string open_file_format)
{
	init_session();

	open_file_name = open_file_name;
	open_file_format = open_file_format;
	//session_->load_init_file(open_file_name, open_file_format);
}

void MainWindow::save_session()
{
	/*
	QSettings settings;
	int id = 0;

	// Ignore sessions using the demo device or no device at all
	if (session_->device()) {
		shared_ptr<devices::HardwareDevice> device =
			dynamic_pointer_cast< devices::HardwareDevice >
			(session->device());

		if (device &&
			device->hardware_device()->driver()->name() == "demo")
			continue;

		settings.beginGroup("Session" + QString::number(id++));
		settings.remove("");  // Remove all keys in this group
		session_->save_settings(settings);
		settings.endGroup();
	}

	settings.setValue("sessions", id);
	*/
}

void MainWindow::restore_session()
{
	/*
	QSettings settings;
	int i, session_count;

	session_count = settings.value("sessions", 0).toInt();

	for (i = 0; i < session_count; i++) {
		settings.beginGroup("Session" + QString::number(i));
		shared_ptr<Session> session = add_session();
		session->restore_settings(settings);
		settings.endGroup();
	}
	*/
}

void MainWindow::remove_session()
{
	// Stop capture while the session still exists so that the UI can be
	// updated in case we're currently running. If so, this will schedule a
	// call to our on_capture_state_changed() slot for the next run of the
	// event loop. We need to have this executed immediately or else it will
	// be dismissed since the session object will be deleted by the time we
	// leave this method and the event loop gets a chance to run again.
	session_->stop_capture();
	QApplication::processEvents();

	/*
	for (shared_ptr<view::ViewBase> view : session_->views())
		remove_view(view);
	*/

	// Remove the session from our list of sessions (which also destroys it)
	//sessions_.remove_if([&](shared_ptr<Session> s) {
	//	return s == session_; });
}

shared_ptr<devices::Device> MainWindow::add_view(
	shared_ptr<devices::HardwareDevice> device)
{
	views::ViewType type;
	QString title = QString::fromStdString(device->sr_hardware_device()->model());

	const auto keys = device->sr_hardware_device()->driver()->config_keys();
	if (keys.count(sigrok::ConfigKey::POWER_SUPPLY))
		type = views::ViewTypeSource;
	else if (keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD))
		type = views::ViewTypeSink;
	else
		type = views::ViewTypeMeasurement;

	QMainWindow *window = new QMainWindow();
	window->setWindowFlags(Qt::Widget);  // Remove Qt::Window flag

	int index = tabWidget->addTab(window, title);
	tabWidget->setCurrentIndex(index);

	window->setDockNestingEnabled(true);

	/*
	shared_ptr<views::ViewBase> main_view =
		add_view(name, views::ViewTypeTrace, *session_);
	*/

	session_->add_device(device);
	device_windows_[device] = window;
	last_focused_device_ = device;

	if (type == views::ViewTypeSource) {
		views::SourceView *sourceViewTab = new views::SourceView(device, window);
		window->setCentralWidget(sourceViewTab);
	} else if (type == views::ViewTypeSink) {
		views::SinkView *sinkViewTab = new views::SinkView(device, window);
		window->setCentralWidget(sinkViewTab);
	} else if (type == views::ViewTypeMeasurement) {
	} else if (type == views::ViewTypeGraph) {
	}

	return device;
}

void MainWindow::remove_view(shared_ptr<devices::HardwareDevice> device)
{
	device = device;
}


void MainWindow::setupUi()
{
	this->resize(724, 444);

	QIcon icon;
	icon.addFile(QStringLiteral(":/icons/smuview.ico"), QSize(), QIcon::Normal, QIcon::Off);
	setWindowIcon(icon);
	actionExit = new QAction(this);
	QIcon icon1;
	icon1.addFile(QStringLiteral(":/icons/application-exit.png"), QSize(), QIcon::Normal, QIcon::Off);
	actionExit->setIcon(icon1);
	actionAbout = new QAction(this);
	QIcon icon2;
	icon2.addFile(QStringLiteral(":/icons/information.svg"), QSize(), QIcon::Normal, QIcon::Off);
	actionAbout->setIcon(icon2);
	actionRun = new QAction(this);
	QIcon icon3;
	icon3.addFile(QStringLiteral(":/icons/status-red.svg"), QSize(), QIcon::Normal, QIcon::Off);
	icon3.addFile(QStringLiteral(":/icons/status-green.svg"), QSize(), QIcon::Normal, QIcon::On);
	icon3.addFile(QStringLiteral(":/icons/status-grey.svg"), QSize(), QIcon::Disabled, QIcon::Off);
	icon3.addFile(QStringLiteral(":/icons/status-grey.svg"), QSize(), QIcon::Disabled, QIcon::On);
	actionRun->setIcon(icon3);
	actionAddView = new QAction(this);
	QIcon icon4;
	icon4.addFile(QStringLiteral(":/icons/window-new.png"), QSize(), QIcon::Normal, QIcon::Off);
	actionAddView->setIcon(icon4);
	centralWidget = new QWidget(this);
	QHBoxLayout *horizontalLayout = new QHBoxLayout(centralWidget);
	horizontalLayout->setSpacing(2);
	horizontalLayout->setContentsMargins(11, 11, 11, 11);
	horizontalLayout->setContentsMargins(2, 2, 2, 2);

	tabWidget = new QTabWidget(centralWidget);

	horizontalLayout->addWidget(tabWidget);

	this->setCentralWidget(centralWidget);
	mainToolBar = new QToolBar(this);
	this->addToolBar(Qt::TopToolBarArea, mainToolBar);
	statusBar = new QStatusBar(this);
	this->setStatusBar(statusBar);

	mainToolBar->addAction(actionAddView);
	mainToolBar->addAction(actionRun);
	mainToolBar->addSeparator();
	mainToolBar->addAction(actionAbout);
	mainToolBar->addAction(actionExit);

	retranslateUi();

	tabWidget->setCurrentIndex(1);

	//QMetaObject::connectSlotsByName(SmuView);
}

void MainWindow::retranslateUi()
{
	this->setWindowTitle(QApplication::translate("SmuView", "SmuView", Q_NULLPTR));
	actionExit->setText(QApplication::translate("SmuView", "Exit", Q_NULLPTR));
	actionAbout->setText(QApplication::translate("SmuView", "About", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
	actionAbout->setToolTip(QApplication::translate("SmuView", "About", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
	actionRun->setText(QApplication::translate("SmuView", "Run", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
	actionRun->setToolTip(QApplication::translate("SmuView", "Run/Stop", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
	actionAddView->setText(QApplication::translate("SmuView", "Add View", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
	actionAddView->setToolTip(QApplication::translate("SmuView", "Add View", Q_NULLPTR));
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

void MainWindow::on_capture_state_changed(int)
{
	int state = session_->get_capture_state();

	const QIcon *icons[] = {&icon_grey_, &icon_red_, &icon_green_};
	actionRun->setIcon(*icons[state]);
	actionRun->setText((state == sv::Session::Stopped) ?
		tr("Run") : tr("Stop"));
}

void MainWindow::on_actionAddView_triggered()
{
	// Stop any currently running capture session
	session_->stop_capture();

	dialogs::Connect dlg(this, device_manager_);

	if (dlg.exec())
		add_view(dlg.get_selected_device());
}

void MainWindow::on_actionRun_triggered()
{
	switch (session_->get_capture_state()) {
	case Session::Stopped:
		session_->start_capture([&](QString message) {
			session_error("Capture failed", message); });
		break;
	case Session::AwaitingTrigger:
	case Session::Running:
		session_->stop_capture();
		break;
	}
}

} // namespace sv

