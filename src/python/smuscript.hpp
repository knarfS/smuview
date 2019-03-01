/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef PYTHON_SMUSCRIPT_HPP
#define PYTHON_SMUSCRIPT_HPP

#include <string>
#include <thread>

namespace sv {

class Session;

namespace python {

class SmuScript
{

public:
	SmuScript(Session &session);

	void run(std::string file_name);
	void stop();

private:
	void script_thread_proc(/*function<void (const QString)> error_handler*/);

	Session &session_;
	std::string script_file_name_;
	std::thread script_thread_;

};

} // namespace python
} // namespace sv

#endif // PYTHON_SMUSCRIPT_HPP
