/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <assert.h>
#include <mutex>

#include <QDebug>

#include "session.hpp"
#include "data/signalbase.hpp"
#include "devices/device.hpp"
#include "devices/hardwaredevice.hpp"

using std::lock_guard;
using std::mutex;

namespace sigrok {
class Context;
}

namespace sv {

shared_ptr<sigrok::Context> Session::sr_context;

Session::Session(DeviceManager &device_manager) :
	device_manager_(device_manager),
	capture_state_(Stopped)
{
	// Set up the session
	sr_session_ = sr_context->create_session();
}

Session::~Session()
{
	stop_capture();
}

DeviceManager& Session::device_manager()
{
	return device_manager_;
}

const DeviceManager& Session::device_manager() const
{
	return device_manager_;
}

/*
shared_ptr<sigrok::Session> Session::session() const
{
	if (!device_)
		return shared_ptr<sigrok::Session>();
	return device_->session();
}
*/

void Session::save_settings(QSettings &settings) const
{
	(QSettings)&settings;

	// TODO: Remove all signal data from settings?
}

void Session::restore_settings(QSettings &settings)
{
	(QSettings)&settings;

	// TODO: Restore all signal data from settings?
}

void Session::add_device(shared_ptr<devices::HardwareDevice> device)
{
	assert(device);

	try {
		device->open();
	} catch (const QString &e) {
		qWarning() << e;
		device.reset();
	}

	sr_session_->add_device(device->sr_device());

	if (device->sr_device()) {
		sr_session_->add_datafeed_callback([=]
			(shared_ptr<sigrok::Device> sr_device, shared_ptr<sigrok::Packet> sr_packet) {
				device->data_feed_in(sr_device, sr_packet);
			});
	}

	devices_.push_back(device);

	/*
	signals_changed();
	device_changed();
	*/
}

void Session::remove_device(shared_ptr<devices::HardwareDevice> device)
{
	if (device)
		device->close();

	// TODO
	//devices_.remove(device);
}

void Session::set_capture_state(capture_state state)
{
	bool changed;

	{
		lock_guard<mutex> lock(sampling_mutex_);
		changed = capture_state_ != state;
		capture_state_ = state;
	}

	if (changed) {
		capture_state_changed(state);
	}
}

Session::capture_state Session::get_capture_state() const
{
	lock_guard<mutex> lock(sampling_mutex_);
	return capture_state_;
}

void Session::start_capture(function<void (const QString)> error_handler)
{
	if (devices_.size() == 0) {
		error_handler(tr("No active device set, can't start acquisition."));
		return;
	}

	stop_capture();

	// Check that at least one channel is enabled
	/*
	const shared_ptr<sigrok::Device> sr_dev = device_->device();
	if (sr_dev) {
		const auto channels = sr_dev->channels();
		if (!any_of(channels.begin(), channels.end(),
			[](shared_ptr<Channel> channel) {
				return channel->enabled(); })) {
			error_handler(tr("No channels enabled."));
			return;
		}
	}
	*/

	// Clear signal data
	/*
	for (const shared_ptr<data::SignalData> d : all_signal_data_)
		d->clear();
	*/

	// Revert name back to default name (e.g. "Session 1") for real devices
	// as the (possibly saved) data is gone. File devices keep their name.
	/*
	shared_ptr<devices::HardwareDevice> hw_device =
		dynamic_pointer_cast< devices::HardwareDevice >(device_);

	if (hw_device) {
		name_ = default_name_;
		name_changed();
	}
	*/

	// Begin the session
	sampling_thread_ = std::thread(
		&Session::sample_thread_proc, this, error_handler);
}

void Session::stop_capture()
{
	if (get_capture_state() != Stopped) {
		sr_session_->stop();
		set_capture_state(Stopped);
	}

	// Check that sampling stopped
	if (sampling_thread_.joinable())
		sampling_thread_.join();
}

// TODO: signal/slot??
void Session::add_signal(shared_ptr<data::SignalBase> signalbase)
{
	all_signals_.insert(signalbase);
}

void Session::sample_thread_proc(function<void (const QString)> error_handler)
{
	assert(error_handler);

	out_of_memory_ = false;

	try {
		sr_session_->start();
	} catch (sigrok::Error e) {
		error_handler(e.what());
		return;
	}

	set_capture_state(Running);

	try {
		sr_session_->run();
	} catch (sigrok::Error e) {
		error_handler(e.what());
		set_capture_state(Stopped);
		return;
	}

	set_capture_state(Stopped);

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

void Session::free_unused_memory()
{
	/* TODO: to device
	for (shared_ptr<data::SignalData> data : all_signal_data_) {
		const vector< shared_ptr<data::Segment> > segments = data->segments();

		for (shared_ptr<data::Segment> segment : segments) {
			segment->free_unused_memory();
		}
	}
	*/
}

} // namespace sv
