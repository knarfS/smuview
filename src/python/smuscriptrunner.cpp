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
	session_(session)
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
}

void SmuScriptRunner::script_thread_proc()
{
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
		qWarning() << "SmuScriptRunner::script_thread_proc(): ex = " << ex.what();
		Q_EMIT script_error(QString(ex.what())); // TODO: script_error() not set atm!
	}

	// TODO / FIXME: When a add_*_view() is not completed at MainThread and
	// script_thread_proc() is finished, SmuView will crash (some object is
	// missing? BaseTab signal? channel?).
	// Workaround: time.sleep(3) in the smuscript
	//
	// Sometimes when many vies are added (completed), SmuView will crash
	// anyways, even after time.sleep(10)

}

} // namespace python
} // namespace sv

