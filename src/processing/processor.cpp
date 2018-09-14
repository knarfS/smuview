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

#include <cassert>

#include <QDateTime>
#include <QDebug>
#include <QString>

#include "processor.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/processing/baseblock.hpp"

using std::shared_ptr;

namespace sv {
namespace processing {

Processor::Processor() :
	processor_state_(processor_state::Stopped)
{
}

Processor::~Processor()
{
}

void Processor::start(function<void (const QString)> error_handler)
{
	if (processor_state_ == processor_state::Started)
		stop();

	processor_start_timestamp_ =
		QDateTime::currentMSecsSinceEpoch() / (double)1000;

	// Start processing
	processor_thread_ = std::thread(
		&Processor::processor_thread_proc, this, error_handler);

	processor_state_ = processor_state::Started;

	processor_thread_.detach();
}

void Processor::pause()
{
	if (processor_state_ != processor_state::Started)
		return;

	processor_state_ = processor_state::Stopped;
}

void Processor::stop()
{
	if (processor_state_ == processor_state::Stop ||
			processor_state_ == processor_state::Stopped)
		return;

	processor_state_ = processor_state::Stop;

	// Check that processing stopped
	if (processor_thread_.joinable())
		processor_thread_.join();

	processor_state_ = processor_state::Stopped;
}

void Processor::add_block_to_process(shared_ptr<BaseBlock> block)
{
	// TODO: sync and/or lock modification in gui
	processing_blocks_.push_back(block);
}

bool Processor::is_running() const
{
	return processor_state_ == processor_state::Started;
}

void Processor::processor_thread_proc(
	function<void (const QString)> error_handler)
{
	assert(error_handler);

	processor_state_ = processor_state::Started;
	// TODO: use std::chrono / std::time
	processor_start_timestamp_ =
		QDateTime::currentMSecsSinceEpoch() / (double)1000;
	Q_EMIT processor_start_timestamp_changed(processor_start_timestamp_);
	Q_EMIT processor_started();
	qWarning()
		<< "Start processor at aquisition_start_timestamp_ = "
		<< util::format_time_date(processor_start_timestamp_);

	// Run each block of each "run at startup" sub process
	for (auto block : processing_blocks_) {
		try {
			if (processor_state_ == processor_state::Stop)
				break;

			// TODO: central preinit?
			block->init();
			block->run();
		}
		catch (sigrok::Error &e) {
			processor_state_ = processor_state::Stopped;
			Q_EMIT processor_finished();
			error_handler(e.what());
			return;
		}
	}

	processor_state_ = processor_state::Stopped;
	Q_EMIT processor_finished();
	qWarning() << "Processor finished";
}

} // namespace processing
} // namespace sv
