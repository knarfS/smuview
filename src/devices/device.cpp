/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2015 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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
#include <glib.h>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDateTime>
#include <QDebug>
#include <QString>

#include "device.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"

using std::bad_alloc;
using std::dynamic_pointer_cast;
using std::lock_guard;
using std::make_shared;
using std::pair;
using std::set;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::vector;
using std::unique_ptr;

namespace sv {
namespace devices {

Device::Device(const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::Device> sr_device):
	sr_context_(sr_context),
	sr_device_(sr_device),
	device_open_(false),
	aquisition_start_timestamp_(0.)
{
	// Set up the session
	sr_session_ = sv::Session::sr_context->create_session();
}

Device::~Device()
{
	if (sr_session_) {
		sr_session_->stop();
		sr_session_->remove_datafeed_callbacks();
	}
}

shared_ptr<sigrok::Device> Device::sr_device() const
{
	return sr_device_;
}

void Device::open(function<void (const QString)> error_handler)
{
	if (device_open_)
		close();

	try {
		sr_device_->open();
	}
	catch (const sigrok::Error &e) {
		throw QString(e.what());
	}

	// Add device to session (do this in constructor??)
	sr_session_->add_device(sr_device_);

	sr_session_->add_datafeed_callback([=]
		(shared_ptr<sigrok::Device> sr_device, shared_ptr<sigrok::Packet> sr_packet) {
			data_feed_in(sr_device, sr_packet);
		});

	device_open_ = true;

	// Start aquisition
	aquisition_thread_ = std::thread(
		&Device::aquisition_thread_proc, this, error_handler);

	aquisition_state_ = aquisition_state::Running;
}

void Device::close()
{
	if (!device_open_)
		return;

	sr_session_->remove_datafeed_callbacks();

	if (aquisition_state_ != aquisition_state::Stopped) {
		sr_session_->stop();
		aquisition_state_ = aquisition_state::Stopped;
	}

	// Check that sampling stopped
	if (aquisition_thread_.joinable())
		aquisition_thread_.join();

	sr_session_->remove_devices();
	sr_device_->close();
	device_open_ = false;
}

void Device::free_unused_memory()
{
	/* TODO
	for (shared_ptr<data::BaseData> data : all_signal_data_) {
		const vector< shared_ptr<data::Segment> > segments = data->segments();

		for (shared_ptr<data::Segment> segment : segments) {
			segment->free_unused_memory();
		}
	}
	*/
}

void Device::data_feed_in(shared_ptr<sigrok::Device> sr_device,
	shared_ptr<sigrok::Packet> sr_packet)
{
	/*
	qWarning() << "data_feed_in(): sr_packet->type()->id() = " << sr_packet->type()->id();
	qWarning() << "data_feed_in(): sr_device->model() = "
		<< QString::fromStdString(sr_device->model())
		<< ", this->model() = " << QString::fromStdString(sr_device_->model());
	*/

	assert(sr_device);
	assert(sr_packet);

	if (sr_device != sr_device_)
		return;

	switch (sr_packet->type()->id()) {
	case SR_DF_HEADER:
		//qWarning() << "data_feed_in(): SR_DF_HEADER";
		feed_in_header();
		break;

	case SR_DF_META:
		//qWarning() << "data_feed_in(): SR_DF_META";
		feed_in_meta(
			dynamic_pointer_cast<sigrok::Meta>(sr_packet->payload()));
		break;

	case SR_DF_TRIGGER:
		//qWarning() << "data_feed_in(): SR_DF_TRIGGER";
		feed_in_trigger();
		break;

	case SR_DF_LOGIC:
		//qWarning() << "data_feed_in(): SR_DF_LOGIC";
		try {
			feed_in_logic(
				dynamic_pointer_cast<sigrok::Logic>(sr_packet->payload()));
		} catch (bad_alloc) {
			out_of_memory_ = true;
			// TODO: sr_session->stop();
		}
		break;

	case SR_DF_ANALOG:
		//qWarning() << "data_feed_in(): SR_DF_ANALOG";
		try {
			feed_in_analog(
				dynamic_pointer_cast<sigrok::Analog>(sr_packet->payload()));
		} catch (bad_alloc) {
			out_of_memory_ = true;
			// TODO: sr_session->stop();
		}
		break;

	case SR_DF_FRAME_BEGIN:
		//qWarning() << "data_feed_in(): SR_DF_FRAME_BEGIN";
		feed_in_frame_begin();
		break;

	case SR_DF_FRAME_END:
		//qWarning() << "data_feed_in(): SR_DF_FRAME_END";
		feed_in_frame_end();
		break;

	case SR_DF_END:
		//qWarning() << "data_feed_in(): SR_DF_END";
		// Strictly speaking, this is performed when a frame end marker was
		// received, so there's no point doing this again. However, not all
		// devices use frames, and for those devices, we need to do it here.
		{
			lock_guard<recursive_mutex> lock(data_mutex_);
		}
		break;

	default:
		break;
	}
}

void Device::aquisition_thread_proc(
	function<void (const QString)> error_handler)
{
	assert(error_handler);

	out_of_memory_ = false;

	try {
		sr_session_->start();
	} catch (sigrok::Error e) {
		error_handler(e.what());
		return;
	}

	aquisition_state_ = aquisition_state::Running;
	// TODO: use std::chrono / std::time
	aquisition_start_timestamp_ =
		QDateTime::currentMSecsSinceEpoch() / (double)1000;
	Q_EMIT aquisition_start_timestamp_changed(aquisition_start_timestamp_);

	qWarning() << "Start aquisition for " << short_name() <<
		",  aquisition_start_timestamp_ = " << aquisition_start_timestamp_;

	try {
		sr_session_->run();
	} catch (sigrok::Error e) {
		error_handler(e.what());
		aquisition_state_ = aquisition_state::Stopped;
		return;
	}

	aquisition_state_ = aquisition_state::Stopped;

	// Optimize memory usage
	free_unused_memory();

	/*
	// We now have unsaved data unless we just "captured" from a file
	shared_ptr<devices::File> file_device =
		dynamic_pointer_cast<devices::File>(device_);

	if (!file_device)
		data_saved_ = false;
	*/

	if (out_of_memory_)
		error_handler(tr("Out of memory, acquisition stopped."));
}

} // namespace devices
} // namespace sv
