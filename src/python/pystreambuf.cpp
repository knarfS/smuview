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

#include <mutex>
#include <string>
#include <vector>

#include <Python.h>
#include <pyerrors.h>

#include "pystreambuf.hpp"

namespace sv {
namespace python {

PyStreamBuf::PyStreamBuf(const std::string &encoding, const std::string &errors) :
	py_closed(false),
	py_encoding(encoding),
	py_errors(errors)
{
}

PyStreamBuf::~PyStreamBuf()
{
	py_close();
}

void PyStreamBuf::py_close()
{
	std::lock_guard<std::mutex> lock(mutex_);

	// output anything that is left
	if (!string_.empty()) {
		Q_EMIT send_string(string_);
		string_.erase(string_.begin(), string_.end());
	}

	py_closed = true;
}

int PyStreamBuf::py_fileno()
{
	PyErr_SetString(PyExc_OSError,
		"PyStreamBuf has no underlying file descriptor!");
	return -1;
}

void PyStreamBuf::py_flush()
{
	std::lock_guard<std::mutex> lock(mutex_);

	Q_EMIT send_string(string_);
	string_.erase(string_.begin(), string_.end());
}

bool PyStreamBuf::py_isatty()
{
	return false;
}

bool PyStreamBuf::py_readable()
{
	return false;
}

std::vector<std::string> PyStreamBuf::py_readlines(int hint)
{
	(void)hint;
	PyErr_SetString(PyExc_OSError, "PyStreamBuf is write only!");
	return std::vector<std::string>();
}

bool PyStreamBuf::py_seekable()
{
	return false;
}

int PyStreamBuf::py_truncate(int size)
{
	(void)size;
	PyErr_SetString(PyExc_OSError, "PyStreamBuf is not seekable!");
	return 0;
}

bool PyStreamBuf::py_writable()
{
	return true;
}

void PyStreamBuf::py_writelines(std::vector<std::string> lines)
{
	if (py_closed)
		PyErr_SetString(PyExc_ValueError, "PyStreamBuf is already closed!");

	for (const auto &s : lines) {
		py_write(s);
	}
}

void PyStreamBuf::py_del()
{
	py_close();
}

std::string PyStreamBuf::py_read(int size)
{
	(void)size;
	PyErr_SetString(PyExc_OSError, "PyStreamBuf is write only!");
	return "";
}

std::string PyStreamBuf::py_readline(int size)
{
	(void)size;
	PyErr_SetString(PyExc_OSError, "PyStreamBuf is write only!");
	return "";
}

int PyStreamBuf::py_seek(int offset, int whence)
{
	(void)offset;
	(void)whence;
	PyErr_SetString(PyExc_OSError, "PyStreamBuf is not seekable!");
	return 0;
}

int PyStreamBuf::py_tell()
{
	PyErr_SetString(PyExc_OSError, "PyStreamBuf is not seekable!");
	return 0;
}

int PyStreamBuf::py_write(std::string s)
{
	if (py_closed)
		PyErr_SetString(PyExc_ValueError, "PyStreamBuf is already closed!");

	std::lock_guard<std::mutex> lock(mutex_);

	string_.append(s);
	size_t pos = 0;
	while (pos != std::string::npos) {
		pos = string_.find('\n');
		if (pos != std::string::npos) {
			std::string tmp(string_.begin(), string_.begin() + pos);
			Q_EMIT send_string(tmp);
			string_.erase(string_.begin(), string_.begin() + pos + 1);
		}
	}

	return s.size();
}

} // namespace python
} // namespace sv
