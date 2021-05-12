/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>
#include <string>
#include <thread>

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include <QDebug>
#include <QFileInfo>
#include <QString>

#include "smuscriptrunner.hpp"
#include "src/session.hpp"
#include "src/python/bindings.hpp"
#include "src/python/pystreambuf.hpp"
#include "src/python/pystreamredirect.hpp"
#include "src/python/uihelper.hpp"
#include "src/python/uiproxy.hpp"

using std::make_shared;
using std::string;

using namespace pybind11::literals; // for the ""_a
namespace py = pybind11;

namespace sv {
namespace python {

SmuScriptRunner::SmuScriptRunner(Session &session) :
	session_(session),
	is_running_(false)
{
	ui_helper_ = make_shared<UiHelper>(session_);
}

SmuScriptRunner::~SmuScriptRunner()
{
	/*
	if (script_thread_.joinable())
		script_thread_.join();
	*/
}

void SmuScriptRunner::run(const string &file_name)
{
	if (file_name.length() == 0) {
		Q_EMIT script_error("SmuScriptRunner",
			tr("No script file specified!").toStdString());
		return;
	}

    QFileInfo file_info(QString::fromStdString(file_name));
	if (!file_info.exists() || !file_info.isFile()) {
		Q_EMIT script_error("SmuScriptRunner",
			tr("No valide script file specified!").toStdString());
		return;
	}

	script_file_name_ = file_name;
	script_thread_ = std::thread(&SmuScriptRunner::script_thread_proc, this);
	script_thread_.detach();
}

void SmuScriptRunner::stop()
{
	if (is_running_)
		PyErr_SetInterrupt();
}

bool SmuScriptRunner::is_running()
{
	return is_running_;
}

void SmuScriptRunner::script_thread_proc()
{
	// TODO: mutex?

	qWarning() << "SmuScriptRunner::script_thread_proc() executing " <<
		QString::fromStdString(script_file_name_);

	is_running_ = true;
	Q_EMIT script_started();

	py::scoped_interpreter guard{};

	py::module smuview_module = py::module::import("smuview");

	// Redirect python stdout + stderr
	PyStreamRedirect py_stream_redirect{ shared_from_this() };

	// NOTE: Setting Session and UiProxy as locals does not work!
	// When executing a script, the globals() inside a function are missing the
	// additional stuff like imported modules, function pointer and also
	// everyhthing provided by the locals dict. Setting Session and UiProxy in
	// addition to py::globals() as globals did the trick. See:
	// https://medium.com/just-me-me-programming-life/python-c-and-symbols-4628fb71a257
	UiProxy *ui_proxy = new UiProxy(session_, ui_helper_);
	auto globals = py::dict(
		**py::globals(),
		"Session"_a=py::cast(session_, py::return_value_policy::reference),
		"UiProxy"_a=py::cast(ui_proxy, py::return_value_policy::reference));

	try {
		py::eval_file(script_file_name_, globals);
	}
	catch (py::error_already_set &ex) {
		Q_EMIT send_py_stderr(ex.what());
		Q_EMIT script_error("SmuScriptRunner py::error_already_set", ex.what());
	}

	qWarning() << "SmuScriptRunner::script_thread_proc() has finished!";
	Q_EMIT script_finished();
	is_running_ = false;
}

} // namespace python
} // namespace sv
