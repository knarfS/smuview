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

#ifndef PYTHON_BINDINGS_HPP
#define PYTHON_BINDINGS_HPP

#include "pybind11/pybind11.h"

namespace py = pybind11;

void init_Session(py::module &m);
void init_Device(py::module &m);
void init_Channel(py::module &m);
void init_Signal(py::module &m);
void init_Configurable(py::module &m);
void init_UI(py::module &m);
void init_StreamBuf(py::module &m);
void init_Enums(py::module &m);

#endif // PYTHON_BINDINGS_HPP
