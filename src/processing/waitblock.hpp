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

#ifndef PROCESSING_WAITBLOCK_HPP
#define PROCESSING_WAITBLOCK_HPP

#include <memory>

#include <QObject>

#include "src/processing/baseblock.hpp"

using std::shared_ptr;

namespace sv {

namespace processing {

class WaitBlock :
	public BaseBlock
{
	Q_OBJECT

public:
	WaitBlock(const shared_ptr<Processor> processor);

public:
	void init();
	void run();

	void set_wait_ms(int wait_ms);

private:
	int wait_ms_;

};

} // namespace processing
} // namespace sv

#endif // PROCESSING_WAITBLOCK_HPP
