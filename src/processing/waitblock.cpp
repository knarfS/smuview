/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <chrono>
#include <thread>

#include <QDebug>

#include "waitblock.hpp"

namespace sv {
namespace processing {

WaitBlock::WaitBlock() : BaseBlock(),
	wait_ms_(0)
{
}

void WaitBlock::init()
{
}

void WaitBlock::run()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms_));
}

void WaitBlock::set_wait_ms(int wait_ms)
{
	wait_ms_ = wait_ms;
}

} // namespace processing
} // namespace sv
