/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2014 Martin Ling <martin-sigrok@earth.li>
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

#include <iostream>

#include "application.hpp"
#include "config.h"

using std::cerr;
using std::endl;
using std::exception;

Application::Application(int &argc, char *argv[]) :
	QApplication(argc, argv)
{
	QApplication::setApplicationVersion(SV_VERSION_STRING);
	QApplication::setApplicationName("SmuView");
	QApplication::setOrganizationName("sigrok");
	QApplication::setOrganizationDomain("sigrok.org");
}

bool Application::notify(QObject *receiver, QEvent *event)
{
	try {
		return QApplication::notify(receiver, event);
	}
	catch (exception &e) {
		cerr << "Caught exception: " << e.what() << endl;
		exit(1);
		return false;
	}
}
