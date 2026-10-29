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

#include <atomic>
#include <thread>
#include <vector>
#include <boost/test/unit_test.hpp>

#include <QObject>

#include "src/data/analogtimesignal.hpp"
#include "testutils/signalutil.hpp"

using namespace sv;
using sv::test::testutils::TimeSignalBuilder;

BOOST_AUTO_TEST_SUITE(analogtimesignal_concurrency_test)

/*
 * Mostly useful under TSan.
 * Tests for torn pair (total_digits, sr_digits)
 */
BOOST_AUTO_TEST_CASE(digits_changed_are_consistent)
{
	auto sig = TimeSignalBuilder().build();
	QObject ctx;
	std::atomic<int> torn{0}, emitted{0};

	// DirectConnection: the lambda runs in the pushing thread
	QObject::connect(sig.get(), &data::AnalogTimeSignal::digits_changed,
		&ctx, [&](int total, int sr) {
			++emitted;
			if (total != sr)
				++torn;
		},
		Qt::DirectConnection);

	std::vector<std::thread> writers;
	for (int t = 0; t < 4; ++t) {
		writers.emplace_back([&, t] {
			for (int i = 0; i < 20000; ++i) {
				const int d = 1 + (t * 7 + i) % 9;
				sig->push_sample(i, t + i * 1e-3, d, d);
			}
		});
	}

	for (auto &w : writers)
		w.join();

	BOOST_CHECK_GT(emitted.load(), 0);
	BOOST_CHECK_EQUAL(torn.load(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
