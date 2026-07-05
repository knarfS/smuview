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
#include <unistd.h>

#include <libsigrokcxx/libsigrokcxx.hpp>
#include <pybind11/embed.h>

#include <QDateTime>
#include <QDebug>
#include <QLabel>
#include <QSettings>

#include "config.h"
#include "src/application.hpp"
#include "src/devicemanager.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/mainwindow.hpp"
#include "src/ui/dialogs/aboutdialog.hpp"
#include "src/ui/tabs/smuscripttab.hpp"

#ifdef ENABLE_SIGNALS
#include "signalhandler.hpp"
#endif

#ifdef _WIN32
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
		/* Disable cmd line options i and I
		"  -i, --input-file           Load input from file\n"
		"  -I, --input-format         Input format\n"
		*/
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
	auto context = sigrok::Context::create();
	context->set_log_level(sigrok::LogLevel::SPEW);
	auto session = init_session(context, {}, true, true);

	// Self test for the GUI, using the about dialog.
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
	qInfo() << "About dialog text: " << text;

	// Self test for the embedded python interpreter
	py::scoped_interpreter guard{};
	py::dict locals;
	py::exec(R"(
		import sys
		py_version = sys.version
		message = "Embedded python version: {}!".format(py_version)
		print(message)
	)", py::globals(), locals);
	string py_message = locals["message"].cast<std::string>();
	if (py_message.rfind("Embedded python version: 3.", 0) != 0) {
		qCritical() << "Could not find expected string in python message!";
		return 1;
	}

	qInfo() << "Selftest passed.";
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	shared_ptr<sigrok::Context> context;
	int loglevel = -1;
	vector<string> drivers;
	//string open_file;
	//string open_file_format;
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
			/* Disable cmd line options i and I
			{ "input-file", required_argument, nullptr, 'i' },
			{ "input-format", required_argument, nullptr, 'I' },
			*/
			{ nullptr, 0, nullptr, 0 }
		};

		/* Disable cmd line options i and I
		const int arg_char = getopt_long(argc, argv,
			"l:Vhc?d:i:I:", long_options, nullptr);
		*/
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

		/* Disable cmd line options i and I
		case 'i':
			open_file = optarg;
			break;

		case 'I':
			open_file_format = optarg;
			break;
		*/
		}
	}

	/* Disable cmd line options i and I
	if (argc - optind > 1) {
		fprintf(stderr, "Only one file can be opened.\n");
		return 1;
	}

	if (argc - optind == 1)
		open_file = argv[argc - 1];
	*/

	// Initialise libsigrok
	context = sigrok::Context::create();
	sv::Session::sr_context = context;

	do {
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
		}
	}
	while (false);

	return ret;
}
