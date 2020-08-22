/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef PYTHON_SMUSCRIPTRUNNER_HPP
#define PYTHON_SMUSCRIPTRUNNER_HPP

#include <memory>
#include <string>
#include <thread>

#include <QObject>
#include <QString>

using std::shared_ptr;
using std::string;

namespace sv {

class Session;

namespace python {

class UiHelper;

class SmuScriptRunner :
	public QObject,
	public std::enable_shared_from_this<SmuScriptRunner>
{
	Q_OBJECT

public:
	explicit SmuScriptRunner(Session &session);
	~SmuScriptRunner();

	void run(const std::string &file_name);
	void stop();
	bool is_running();

private:
	void script_thread_proc();

	Session &session_;
	shared_ptr<UiHelper> ui_helper_;
	string script_file_name_;
	std::thread script_thread_;
	bool is_running_;

Q_SIGNALS:
	void script_error(const std::string &sender, const std::string &msg);
	void script_started();
	void script_finished();
	void send_py_stdout(const std::string &text);
	void send_py_stderr(const std::string &text);

};

} // namespace python
} // namespace sv

#endif // PYTHON_SMUSCRIPTRUNNER_HPP
