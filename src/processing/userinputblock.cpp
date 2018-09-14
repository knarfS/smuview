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

#include <mutex>
#include <thread>

#include <QDebug>

#include "userinputblock.hpp"
#include "src/processing/processor.hpp"

using std::mutex;
using std::shared_ptr;
using std::unique_lock;

namespace sv {
namespace processing {

UserInputBlock::UserInputBlock(const shared_ptr<Processor> processor) :
		BaseBlock(processor),
	notified_(false),
	message_("")
{
}

void UserInputBlock::init()
{
}

void UserInputBlock::run()
{
	notified_ = false;
	mutex m;
	unique_lock<mutex> lock(m);

	Q_EMIT show_message(message_);
	while (!notified_) {
		cond_var_.wait(lock);
	}
}

void UserInputBlock::set_message(QString message)
{
	message_ = message;
}

void UserInputBlock::on_message_closed()
{
	notified_ = true;
	cond_var_.notify_one();
}

} // namespace processing
} // namespace sv
