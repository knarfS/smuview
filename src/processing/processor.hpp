/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef PROCESSING_PROCESSOR_HPP
#define PROCESSING_PROCESSOR_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include <QObject>
#include <QString>

using std::function;
using std::map;
using std::mutex;
using std::recursive_mutex;
using std::shared_ptr;
using std::vector;

namespace sv {
namespace processing {

class BaseBlock;

class Processor : public QObject
{
	Q_OBJECT

public:
	Processor();
	~Processor();

	enum processor_state {
		Started,
		Paused,
		Stop,
		Stopped
	};

	void start(function<void (const QString)> error_handler);
	void pause();
	void stop();
	void add_block_to_process(shared_ptr<BaseBlock>);
	bool is_running() const;

private:
	std::thread processor_thread_;
	vector<std::thread> sub_threads_;
	vector<shared_ptr<BaseBlock>> processing_blocks_;

	mutable mutex processor_mutex_; //!< Protects access to capture_state_. // TODO
	processor_state processor_state_;
	double processor_start_timestamp_;

	void processor_thread_proc(function<void (const QString)> error_handler);


Q_SIGNALS:
	void processor_start_timestamp_changed(double);
	void processor_started();
	void processor_finished();

};

} // namespace processing
} // namespace sv

#endif // PROCESSING_PROCESSOR_HPP
