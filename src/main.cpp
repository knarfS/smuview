/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <getopt.h>
#include <memory>
#include <qcontainerfwd.h>
#include <unistd.h>

#include <libsigrokcxx/libsigrokcxx.hpp>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QSettings>

#include "src/application.hpp"
#include "src/config.h"
#include "src/devicemanager.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/mainwindow.hpp"
#include "src/ui/dialogs/aboutdialog.hpp"
#include "src/ui/tabs/smuscripttab.hpp"

#ifdef ENABLE_SIGNALS
#include "src/signalhandler.hpp"
#endif

#if defined(_WIN32)
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QSvgPlugin)
#endif

using std::exception;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

namespace py = pybind11;

void usage()
{
	fprintf(stdout,
		"Usage:\n"
		"  %s [OPTIONS] [FILE]\n"
		"\n"
		"Help Options:\n"
		"  -h, -?, --help             Show help option\n"
		"\n"
		"Application Options:\n"
		"  -V, --version              Show release version\n"
		"  -l, --loglevel             Set libsigrok loglevel (0-5, default: 2)\n"
		"  -d, --driver               Specify the device driver(s) to use\n"
		"  -D, --dont-scan            Don't auto-scan for devices, use -d spec only\n"
		"  -s, --script               Specify the SmuScript to load and execute\n"
		"  -c, --clean                Don't restore previous settings on startup\n"
		"  -t, --selftest             Selftest function for CI\n"
		"\n"
		"Examples:\n"
		"  %s --driver tecpel-dmm-8061-ser:conn=/dev/ttyUSB0\n"
		"\n"
		"  %s --driver uni-t-ut61e:conn=1a86.e008\n"
		"\n"
		"  %s --driver voltcraft-k204:conn=/dev/ttyUSB0 \\\n"
		"     --driver uni-t-ut61d:conn=1a86.e008 \\\n"
		"     --driver uni-t-ut61e-ser:conn=/dev/ttyUSB1\n",
		SV_BIN_NAME, SV_BIN_NAME, SV_BIN_NAME, SV_BIN_NAME);
}

shared_ptr<sv::Session> init_session(shared_ptr<sigrok::Context> context,
	const vector<string> &drivers, bool do_scan, bool restore_settings)
{
	sv::Session::sr_context = context;
	sv::SettingsManager::set_restore_settings(restore_settings);

	// Initialize global start timestamp
	// TODO: use std::chrono / std::time
	sv::Session::session_start_timestamp =
		(double)QDateTime::currentMSecsSinceEpoch() / 1000.0;

	// Create the device manager, initialise the drivers
	static sv::DeviceManager device_manager(context, drivers, do_scan);

	// Initialise the session.
	return make_shared<sv::Session>(device_manager);
}

int selftest()
{
	// Initialise libsigrok and the session
	qInfo() << "--- Selftest Application Startup ---";
	auto context = sigrok::Context::create();
	context->set_log_level(sigrok::LogLevel::SPEW);
	auto session = init_session(context, {}, true, true);

	// Self test for the GUI, using the about dialog.
	qInfo() << "--- Selftest GUI ---";
	sv::ui::dialogs::AboutDialog about_dlg(session->device_manager(), nullptr);
	about_dlg.show();
	QApplication::processEvents();

	QLabel *version_info = about_dlg.findChild<QLabel*>("version_info");
	if (!version_info) {
		qCritical() << "Selftest failed, label not found!";
		return 1;
	}
	QString text = version_info->text();
	bool ok = text.contains(QApplication::applicationVersion());
	if (!ok) {
		qCritical() << "About dialog does not contain expected text!";
		return 1;
	}
	qInfo().noquote() << QString("About dialog text: %1").arg(text);

	// Self test the embedded python interpreter
	qInfo() << "--- Selftest Embedded Python ---";
	py::scoped_interpreter guard{};
	py::dict locals;
	py::exec(R"(
		import sys, os
		py_test_msg = f"Embedded python version: {sys.version}"
		py_sys_version = sys.version
		py_sys_executable = sys.executable
		py_sys_prefix = sys.prefix
		py_sys_base_prefix = sys.base_prefix
		py_sys_path = sys.path
		py_os_module_file = os.__file__
	)", py::globals(), locals);

	string py_test_msg = locals["py_test_msg"].cast<string>();
	string py_sys_version = locals["py_sys_version"].cast<string>();
	string py_sys_executable = locals["py_sys_executable"].cast<string>();
	string py_sys_prefix = locals["py_sys_prefix"].cast<string>();
	string py_sys_base_prefix = locals["py_sys_base_prefix"].cast<string>();
	vector<string> py_sys_path = locals["py_sys_path"].cast<vector<string>>();
	QString py_sys_path_str;
	for (const auto &path : py_sys_path)
		py_sys_path_str.append("'").append(QString::fromStdString(path))
			.append("', ");
	string py_os_module_file = locals["py_os_module_file"].cast<string>();

	qInfo().noquote() << QString::fromStdString(py_test_msg);
	qInfo().noquote() << QString("Embedded sys.version: %1").arg(
		QString::fromStdString(py_sys_version));
	qInfo().noquote() << QString("Embedded sys.executable: %1").arg(
		QString::fromStdString(py_sys_executable));
	qInfo().noquote() << QString("Embedded sys.prefix: %1").arg(
		QString::fromStdString(py_sys_prefix));
	qInfo().noquote() << QString("Embedded sys.base_prefix: %1").arg(
		QString::fromStdString(py_sys_base_prefix));
	qInfo().noquote() << QString("Embedded sys.path: %1").arg(py_sys_path_str);
	qInfo().noquote() << QString("Embedded os.__file__: %1").arg(
		QString::fromStdString(py_os_module_file));

	if (py_test_msg.rfind("Embedded python version: 3.", 0) != 0) {
		qCritical() << "Could not find expected string in py_test_msg!";
		return 1;
	}

	// Validate python sys.path
	qInfo() << "--- Selftest Python sys.path ---";
	QString app_dir = QCoreApplication::applicationDirPath();
#if defined(_WIN32)
	QString expected_prefix = app_dir;
#elif defined(__APPLE__)
	QString expected_prefix = QDir::cleanPath(app_dir + "/../Frameworks");
#else // Linux / AppImage
	 QString expected_prefix = QDir::cleanPath(app_dir + "/../lib");
#endif
	qInfo().noquote() << QString("Validating python paths agains '%1'").arg(
		expected_prefix);

	if (!QDir(expected_prefix).exists()) {
		qWarning().noquote() << QString(
				"Application directory '%1' does not exits, skipping sys.path validation")
			.arg(expected_prefix);
	}
	else {
		for (const auto &entry : py_sys_path) {
			QString normalized =
				QDir::fromNativeSeparators(QString::fromStdString(entry));
			if (!normalized.isEmpty() && !normalized.startsWith(expected_prefix)) {
				qCritical().noquote()
					<< QString("sys.path '%1' is outside the app directory")
						.arg(QString::fromStdString(entry));
				return 1;
			}
		}
	}

	// Print some config vars
	qInfo() << "--- Selftest Build Time Variables ---";
	qInfo().noquote() << QString("SV_VERSION_STRING: %1").arg(SV_VERSION_STRING);
	qInfo().noquote() << QString("SV_PYTHON_VERSION: %1").arg(SV_PYTHON_VERSION);
	qInfo() << "----------------";
	qInfo() << "Selftest passed.";
	qInfo() << "----------------";
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	shared_ptr<sigrok::Context> context;
	int loglevel = -1;
	vector<string> drivers;
	bool do_scan = true;
	string script_file;
	bool restore_settings = true;

	Application app(argc, argv);

	// Parse arguments
	while (true) {
		static const struct option long_options[] = {
			{ "help", no_argument, nullptr, 'h' },
			{ "version", no_argument, nullptr, 'V' },
			{ "loglevel", required_argument, nullptr, 'l' },
			{ "driver", required_argument, nullptr, 'd' },
			{ "dont-scan", no_argument, nullptr, 'D' },
			{ "script", required_argument, nullptr, 's' },
			{ "clean", no_argument, nullptr, 'c' },
			{ "selftest", no_argument, nullptr, 't' },
			{ nullptr, 0, nullptr, 0 }
		};

		const int arg_char = getopt_long(argc, argv,
			"h?VDl:d:s:ct", long_options, nullptr);

		if (arg_char == -1)
			break;

		switch (arg_char) {
		case 'h':
		case '?':
			usage();
			return 0;

		case 'V':
			// Print version info
			fprintf(stdout, "%s %s\n", SV_TITLE, SV_VERSION_STRING);
			return 0;

		case 'l':
		{
			loglevel = atoi(optarg);
			if (loglevel >= 5) {
				const QSettings settings;
				qDebug() << "Settings:" << settings.fileName()
					<< "format" << settings.format();
			}
			break;
		}

		case 'd':
			drivers.push_back(optarg);
			break;

		case 'D':
			do_scan = false;
			break;

		case 's':
			script_file = optarg;
			break;

		case 'c':
			restore_settings = false;
			break;

		case 't':
			return selftest();

		}
	}

	// Initialise libsigrok
	context = sigrok::Context::create();
	sv::Session::sr_context = context;

	try {
		// Initialise libsigrok and the session
		context = sigrok::Context::create();
		if (loglevel >= 0)
			context->set_log_level(sigrok::LogLevel::get(loglevel));
		auto session =
			init_session(context, drivers, do_scan, restore_settings);

		// Initialise the main window.
		sv::MainWindow main_window(session->device_manager(), session);
		main_window.show();

		if (!script_file.empty())
			main_window.add_smuscript_tab(script_file)->run_script();

#ifdef ENABLE_SIGNALS
		if (SignalHandler::prepare_signals()) {
			SignalHandler *const handler = new SignalHandler(&main_window);
			QObject::connect(handler, &SignalHandler::int_received,
				&main_window, &sv::MainWindow::close);
			QObject::connect(handler, &SignalHandler::term_received,
				&main_window, &sv::MainWindow::close);
		}
		else {
			qWarning() << "Could not prepare signal handler.";
		}
#endif

		// Run the application
		ret = Application::exec();
	}
	catch (exception &e) {
		qCritical() << "main() failed: " << e.what();
		ret = 1;
	}

	return ret;
}
