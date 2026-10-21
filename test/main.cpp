/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE SmuViewTest

#include <boost/test/included/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <QCoreApplication>

/**
 * This fixture provides a Qt main loop, which is needed to test signals
 */
struct QtApplicationFixture
{
	QtApplicationFixture()
	{
		static char app_name[] = "smuview-test";
		static int argc = 1;
		static char *argv[] = { app_name, nullptr };
		app = new QCoreApplication(argc, argv);
	}

	~QtApplicationFixture()
	{
		delete app;
	}

	QCoreApplication *app = nullptr;
};

BOOST_GLOBAL_FIXTURE(QtApplicationFixture);
