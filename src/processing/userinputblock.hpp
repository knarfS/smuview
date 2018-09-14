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

#ifndef PROCESSING_USERINPUTBLOCK_HPP
#define PROCESSING_USERINPUTBLOCK_HPP

#include <condition_variable>
#include <memory>

#include <QObject>
#include <QString>

#include "src/processing/baseblock.hpp"

using std::condition_variable;
using std::shared_ptr;

namespace sv {

namespace processing {

class UserInputBlock :
	public BaseBlock
{
	Q_OBJECT

public:
	UserInputBlock(const shared_ptr<Processor> processor);

public:
	void init();
	void run();

	void set_message(QString);

private:
	condition_variable cond_var_;
	bool notified_;

	QString message_;

public Q_SLOTS:
	void on_message_closed();

Q_SIGNALS:
	void show_message(QString);

};

} // namespace processing
} // namespace sv

#endif // PROCESSING_USERINPUTBLOCK_HPP
