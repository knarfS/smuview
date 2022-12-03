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

#ifndef PYTHON_PYSTREAMBUF_H
#define PYTHON_PYSTREAMBUF_H

#include <mutex>
#include <string>
#include <vector>

#include <QObject>

namespace sv {
namespace python {

/**
 * Buffer that writes to C++ instead of Python.
 */
class PyStreamBuf : public QObject
{
	Q_OBJECT

public:
	PyStreamBuf(const std::string &encoding, const std::string &errors);
	~PyStreamBuf();

	/** True if the stream is closed. */
	bool py_closed;
	/** The name of the encoding that is used. */
	const std::string py_encoding;
	/** The error setting of the decoder or encoder. */
	const std::string py_errors;

	/** Flush and close this stream. */
	void py_close();
	/** Raises an OSError, because PyStreamBuf doesn't use a file descriptor. */
	int py_fileno();
	/** Flush the write buffers of the stream. */
	void py_flush();
	/** Always return False. */
	bool py_isatty();
	/** Always return False. */
	bool py_readable();
	/** Raises an OSError, because PyStreamBuf is write only. */
	std::vector<std::string> py_readlines(int hint = -1);
	/** Always return False. PyStreamBuf is not seekable atm. */
	bool py_seekable();
	/** Raises an OSError, because PyStreamBuf is not seekable. */
	int py_truncate(int size = 0);
	/** Always return True. */
	bool py_writable();
	/** Write a list of lines to the stream. */
	void py_writelines(const std::vector<std::string> &lines);
	/** Prepare for object destruction. */
	void py_del();
	/** Raises an OSError, because PyStreamBuf is write only. */
	std::string py_read(int size = -1);
	/** Raises an OSError, because PyStreamBuf is write only. */
	std::string py_readline(int size = -1);
	/** Raises an OSError, because PyStreamBuf is not seekable. */
	int py_seek(int offset, int whence = SEEK_SET);
	/** Raises an OSError, because PyStreamBuf is not seekable. */
	int py_tell();
	/**
	 * Write the string `str` to the stream and return the number of
	 * characters written.
	 */
	int py_write(const std::string &str);

private:
	std::string string_;
	std::mutex mutex_;

Q_SIGNALS:
	void send_string(const std::string &text);

};

} // namespace python
} // namespace sv

#endif // PYTHON_PYSTREAMBUF_H
