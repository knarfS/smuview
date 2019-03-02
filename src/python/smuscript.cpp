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

#include "smuscript.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/hardwaredevice.hpp"

using namespace pybind11::literals; // for the ""_a
namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(smuview, m) {
    py::class_<sv::Session>(m, "Session")
        .def("devices", &sv::Session::devices);

	py::class_<sv::devices::BaseDevice, std::shared_ptr<sv::devices::BaseDevice>>(m, "BaseDevice")
		.def("id", &sv::devices::BaseDevice::id);

	//py::class_<sv::devices::HardwareDevice, std::shared_ptr<sv::devices::HardwareDevice>>(m, "HardwareDevice")
	//	.def("name", &sv::devices::HardwareDevice::name);
}

namespace sv {
namespace python {

SmuScript::SmuScript(Session &session) :
	session_(session)
{
}

void SmuScript::run(std::string file_name)
{
	script_file_name_ = file_name;

	script_thread_ = std::thread(&SmuScript::script_thread_proc, this);
	script_thread_.join();
}

void SmuScript::stop()
{
}

void SmuScript::script_thread_proc()
{
	py::scoped_interpreter guard{};

    auto module = py::module::import("smuview");
	auto locals = py::dict(
		"Session"_a=py::cast(session_, py::return_value_policy::reference));

    py::eval_file(script_file_name_, py::globals(), locals);
}

} // namespace python
} // namespace sv

