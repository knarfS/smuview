/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDateTime>
#include <QDebug>
#include <QSettings>

#include "config.h"
#include "src/application.hpp"
#include "src/devicemanager.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/mainwindow.hpp"
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
			/* Disable cmd line options i and I
			{ "input-file", required_argument, nullptr, 'i' },
			{ "input-format", required_argument, nullptr, 'I' },
			*/
			{ nullptr, 0, nullptr, 0 }
		};

		/* Disable cmd line options i and I
		const int c = getopt_long(argc, argv,
			"l:Vhc?d:i:I:", long_options, nullptr);
		*/
		const int c = getopt_long(argc, argv,
			"h?VDl:d:s:c", long_options, nullptr);

		if (c == -1)
			break;

		switch (c) {
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
			if (loglevel >= 0)
				context->set_log_level(sigrok::LogLevel::get(loglevel));

			sv::SettingsManager::set_restore_settings(restore_settings);

			// Initialize global start timestamp
			// TODO: use std::chrono / std::time
			sv::Session::session_start_timestamp =
				(double)QDateTime::currentMSecsSinceEpoch() / (double)1000;

			// Create the device manager, initialise the drivers
			sv::DeviceManager device_manager(context, drivers, do_scan);

			// Initialise the session.
			auto session = make_shared<sv::Session>(device_manager);

			// Initialise the main window.
			sv::MainWindow w(device_manager, session);
			w.show();

			if (!script_file.empty())
				w.add_smuscript_tab(script_file)->run_script();

#ifdef ENABLE_SIGNALS
			if (SignalHandler::prepare_signals()) {
				SignalHandler *const handler =
					new SignalHandler(&w);
				QObject::connect(handler,
					SIGNAL(int_received()),
					&w, SLOT(close()));
				QObject::connect(handler,
					SIGNAL(term_received()),
					&w, SLOT(close()));
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
