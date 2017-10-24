/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#include <cstdint>
#include <getopt.h>
#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>
#include <QSettings>

#ifdef _WIN32
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QSvgPlugin)
#endif

#include "config.h"
#include "src/application.hpp"
#include "src/devicemanager.hpp"
#include "src/session.hpp"
#include "src/mainwindow.hpp"

#ifdef ENABLE_SIGNALS
#include "signalhandler.hpp"
#endif

using std::exception;
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
		"  -l, --loglevel             Set libsigrok loglevel\n"
		"  -d, --driver               Specify the device driver(s) to use\n"
		"  -i, --input-file           Load input from file\n"
		"  -I, --input-format         Input format\n"
		"  -c, --clean                Don't restore previous session on startup\n"
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
	vector<string> drivers;
	string open_file, open_file_format;
	bool restore_session = true;

    Application app(argc, argv);

	// Parse arguments
	while (true) {
		static const struct option long_options[] = {
			{"help", no_argument, nullptr, 'h'},
			{"version", no_argument, nullptr, 'V'},
			{"loglevel", required_argument, nullptr, 'l'},
			{"driver", required_argument, nullptr, 'd'},
			{"input-file", required_argument, nullptr, 'i'},
			{"input-format", required_argument, nullptr, 'I'},
			{"clean", no_argument, nullptr, 'c'},
			{nullptr, 0, nullptr, 0}
		};

		const int c = getopt_long(argc, argv,
			"l:Vhc?d:i:I:", long_options, nullptr);
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
			const int loglevel = atoi(optarg);
			context->set_log_level(sigrok::LogLevel::get(loglevel));

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

		case 'i':
			open_file = optarg;
			break;

		case 'I':
			open_file_format = optarg;
			break;

		case 'c':
			restore_session = false;
			break;
		}
	}

	if (argc - optind > 1) {
		fprintf(stderr, "Only one file can be opened.\n");
		return 1;
	}

	if (argc - optind == 1)
		open_file = argv[argc - 1];

	// Initialise libsigrok
	context = sigrok::Context::create();
	sv::Session::sr_context = context;

	do {
		try {
			// Create the device manager, initialise the drivers
			sv::DeviceManager device_manager(context, drivers);

			// Initialise the main window
			sv::MainWindow w(device_manager);
			w.show();

			// TODO: restore session
			if (restore_session) {
				w.restore_session();
			}

			if (!open_file.empty())
				w.init_session_with_file(open_file, open_file_format);
			else
				w.init_default_session();

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
			} else {
				qWarning() <<
					"Could not prepare signal handler.";
			}
#endif

			// Run the application
			ret = app.exec();
		}
		catch (exception e) {
			qDebug() << e.what();
		}

	} while (false);

	return ret;
}
