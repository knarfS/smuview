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

#ifndef PYTHON_PYSTREAMREDIRECT_HPP
#define PYTHON_PYSTREAMREDIRECT_HPP

#include <iostream>
#include <memory>
#include <string>

#include <pybind11/pybind11.h>

#include <QObject>

#include "src/python/pystreambuf.hpp"
#include "src/python/smuscriptrunner.hpp"

using std::shared_ptr;
using std::string;

namespace py = pybind11;

namespace sv {
namespace python {

class __attribute__((visibility("hidden"))) PyStreamRedirect : public QObject
{
	Q_OBJECT

public:
    explicit PyStreamRedirect(shared_ptr<SmuScriptRunner> script_runner) :
		script_runner_(script_runner)
	{
		auto sys_module = py::module::import("sys");
		old_stdout_ = sys_module.attr("stdout");
		old_stderr_ = sys_module.attr("stderr");

		auto locale_module = py::module::import("locale");
		auto default_encoding = locale_module.attr("getpreferredencoding")(false);

		stdout_buf_ = new PyStreamBuf(
			py::str(py::getattr(old_stdout_, "encoding", default_encoding)),
			py::str(py::getattr(old_stdout_, "errors", py::str("strict"))));
		auto py_stdout_buf = py::cast(
			stdout_buf_, py::return_value_policy::reference);
		connect(stdout_buf_, &PyStreamBuf::send_string,
			script_runner_.get(), &SmuScriptRunner::send_py_stdout);

		stderr_buf_ = new PyStreamBuf(
			py::str(py::getattr(old_stderr_, "encoding", default_encoding)),
			py::str(py::getattr(old_stderr_, "errors", py::str("backslashreplace"))));
		auto py_stderr_buf = py::cast(
			stderr_buf_, py::return_value_policy::reference);
		connect(stderr_buf_, &PyStreamBuf::send_string,
			script_runner_.get(), &SmuScriptRunner::send_py_stderr);

		sys_module.attr("stdout") = py_stdout_buf;
		sys_module.attr("stderr") = py_stderr_buf;
	}

	~PyStreamRedirect()
	{
		try {
			auto sys_module = py::module::import("sys");
			sys_module.attr("stdout") = old_stdout_;
			sys_module.attr("stderr") = old_stderr_;
		}
		catch (const py::error_already_set &) {}

		stdout_buf_->py_close();
		stderr_buf_->py_close();

		disconnect(stdout_buf_, &PyStreamBuf::send_string,
			script_runner_.get(), &SmuScriptRunner::send_py_stdout);
		disconnect(stderr_buf_, &PyStreamBuf::send_string,
			script_runner_.get(), &SmuScriptRunner::send_py_stderr);
	}

private:
	shared_ptr<SmuScriptRunner> script_runner_;
	py::object old_stdout_;
	py::object old_stderr_;
	PyStreamBuf *stdout_buf_;
	PyStreamBuf *stderr_buf_;

};

} // namespace python
} // namespace sv

#endif // PYTHON_PYSTREAMREDIRECT_HPP
