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

#include <memory>
#include <thread>
#include <vector>
#include <string>
#include <boost/test/unit_test.hpp>

#include "src/channels/basechannel.hpp"
#include "src/data/datautil.hpp"

using namespace sv;

BOOST_AUTO_TEST_SUITE(basechannel_concurrency_test)

namespace {

void run_in_thread(size_t thread_count, const std::function<void(int)> &fn)
{
	std::vector<std::thread> threads;
	for (size_t i = 0; i < thread_count; ++i)
		threads.emplace_back(fn, i);
	for (auto &t : threads)
		t.join();
}

}
/*
 * Mostly useful under TSan.
 * Asserts all signals were added.
 */
BOOST_AUTO_TEST_CASE(basechannel_add_signal_concurrent)
{
	auto channel = std::make_shared<channels::BaseChannel>(
		nullptr, nullptr, std::set<std::string>{});

	const std::vector<data::Quantity> quantities = {
		data::Quantity::Voltage, data::Quantity::Current, data::Quantity::Power,
		data::Quantity::Count, data::Quantity::Energy, data::Quantity::Gain
	};

	run_in_thread(quantities.size(), [&](int i) {
		channel->add_signal(quantities.at(i), { data::QuantityFlag::DC },
			data::Unit::Volt);
	});

	BOOST_CHECK_EQUAL(channel->signals().size(), quantities.size());
	BOOST_CHECK_EQUAL(channel->signal_map().size(), quantities.size());
}

/*
 * Mostly useful under TSan.
 * Asserts only one signal was added.
 */
BOOST_AUTO_TEST_CASE(basechannel_add_signal_same_mq_concurrent)
{
	auto channel = std::make_shared<channels::BaseChannel>(
		nullptr, nullptr, std::set<std::string>{});

	run_in_thread(50, [&](int) {
		channel->add_signal(data::Quantity::Voltage, { data::QuantityFlag::DC },
			data::Unit::Volt);
	});

	BOOST_CHECK_EQUAL(channel->signals().size(), 1u);
	BOOST_CHECK_EQUAL(channel->signal_map().size(), 1u);
}

BOOST_AUTO_TEST_SUITE_END()
