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

#include <memory>
#include <string>
#include <thread>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include <QDebug>
#include <QString>

#include "smuscriptrunner.hpp"
#include "src/session.hpp"
#include "src/python/bindings.hpp"
#include "src/python/uihelper.hpp"
#include "src/python/uiproxy.hpp"

using std::make_shared;

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

void SmuScriptRunner::run(std::string file_name)
{
	script_file_name_ = file_name;
	std::thread script_thread =
		std::thread(&SmuScriptRunner::script_thread_proc, this);
	script_thread.detach();
}

void SmuScriptRunner::stop()
{
	// TODO: finalize_interpreter crashes...
	//py::finalize_interpreter();
}

bool SmuScriptRunner::is_running()
{
	return is_running_;
}

void SmuScriptRunner::script_thread_proc()
{
	// TODO: mutex?
	is_running_ = true;
	Q_EMIT script_started();

	py::scoped_interpreter guard{};

	UiProxy *ui_proxy = new UiProxy(session_, ui_helper_);
	auto module = py::module::import("smuview");
	auto locals = py::dict(
		"Session"_a=py::cast(session_, py::return_value_policy::reference),
		"UiProxy"_a=py::cast(ui_proxy, py::return_value_policy::reference));

	try {
		py::eval_file(script_file_name_, py::globals(), locals);
	}
	catch (py::error_already_set &ex) {
		Q_EMIT script_error("SmuScriptRunner", ex.what());
	}

	qWarning() << "SmuScriptRunner::script_thread_proc() has finished!";
	Q_EMIT script_finished();
	is_running_ = false;
}

} // namespace python
} // namespace sv

