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

#include <glib.h>
#include <thread>
#include <boost/algorithm/string/join.hpp>

#include <QDateTime>
#include <QDebug>
#include <QString>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "hardwaredevice.hpp"
#include "src/devicemanager.hpp"
#include "src/session.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/device.hpp"
#include "src/data/analogdata.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/basedata.hpp"

using std::bad_alloc;
using std::dynamic_pointer_cast;
using std::lock_guard;
using std::make_shared;
using std::map;
using std::pair;
using std::set;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::vector;
using std::unique_ptr;

using boost::algorithm::join;

namespace sv {
namespace devices {

HardwareDevice::HardwareDevice(
		const shared_ptr<sigrok::Context> &sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device) :
	Device(sr_context),
	device_open_(false)
{
	// TODO: sigrok::Device and not sigrok::HardwareDevice in constructor?? then cast...
	sr_device_ = sr_device;

	// Sigrok Channel Groups + Channel Group Names Map
	map<string, shared_ptr<sigrok::ChannelGroup>> sr_cgs =
		sr_device_->channel_groups();
	map<string, shared_ptr<sigrok::ChannelGroup>>::iterator it = sr_cgs.begin();
	for (; it != sr_cgs.end(); ++it) {
		sr_channel_groups_.push_back(it->second);

		/*
		channel_group_name_signal_map_.insert(
			pair<QString, shared_ptr<data::BaseSignal>>(
				QString::fromStdString(it->first), it->second));
		*/
	}

	vector<shared_ptr<sigrok::Channel>> sr_channels;
	shared_ptr<data::AnalogData> common_time_data_; // TODO: Per channel group?

	const auto sr_keys = sr_device->driver()->config_keys();
	if (sr_keys.count(sigrok::ConfigKey::POWER_SUPPLY)) {
		type_ = HardwareDevice::POWER_SUPPLY;

		if (sr_device_->channel_groups().size() > 0) {
			for (auto channel_group : sr_device_->channel_groups()) {
				configurables_.push_back(
					make_shared<Configurable>(channel_group.second));
				sr_channels.insert(
					sr_channels.end(),
					channel_group.second->channels().begin(),
					channel_group.second->channels().end());
			}
		}
		else {
			configurables_.push_back(make_shared<Configurable>(sr_device_));
			sr_channels = sr_device_->channels();
		}

		// TODO: solve this somehow with the detection of frames....
		// TODO: What if the device has multi channels with a frame around each cg data
		// PPUs DON'T have common time data
		common_time_data_ = nullptr;
	}
	else if (sr_keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD)) {
		type_ = HardwareDevice::ELECTRONIC_LOAD;

		// TODO: POWER_SUPPLY + ELECTRONIC_LOAD common code
		if (sr_device_->channel_groups().size() > 0) {
			for (auto channel_group : sr_device_->channel_groups()) {
				configurables_.push_back(
					make_shared<Configurable>(channel_group.second));
				sr_channels.insert(
					sr_channels.end(),
					channel_group.second->channels().begin(),
					channel_group.second->channels().end());
			}
		}
		else {
			configurables_.push_back(make_shared<Configurable>(sr_device_));
			sr_channels = sr_device_->channels();
		}

		// TODO: solve this somehow with the detection of frames....
		// TODO: What if the device has multi channels with a frame around each cg data
		// Loads have common time data
		common_time_data_ = init_time_data();
	}
	else if (sr_keys.count(sigrok::ConfigKey::MULTIMETER)) {
		type_ = HardwareDevice::MULTIMETER;

		qWarning() << "MM HardwareDevice::MULTIMETER";
		// TODO: common code
		/*
		if (sr_device_->channel_groups().size() > 0) {
			for (auto channel_group : sr_device_->channel_groups()) {
				configurables_.push_back(
					make_shared<Configurable>(channel_group.second));
				sr_channels.insert(
					sr_channels.end(),
					channel_group.second->channels().begin(),
					channel_group.second->channels().end());
			}
		}
		else {
			*/
			configurables_.push_back(make_shared<Configurable>(sr_device_));
			sr_channels = sr_device_->channels();
		//}

		common_time_data_ = nullptr;
	}
	else if (sr_keys.count(sigrok::ConfigKey::DEMO_DEV)) {
		type_ = HardwareDevice::DEMO_DEV;

		qWarning() << "XX HardwareDevice::DEMO_DEV";
		// TODO: common code
		/*
		if (sr_device_->channel_groups().size() > 0) {
			sr_configurable_ = sr_device_->channel_groups()["Analog"];
			for (auto channel_group : sr_device_->channel_groups()) {
				configurables_.push_back(
					make_shared<Configurable>(channel_group.second));
				sr_channels.insert(
					sr_channels.end(),
					channel_group.second->channels().begin(),
					channel_group.second->channels().end());
			}
		}
		else {
			configurables_.push_back(make_shared<Configurable>(sr_device_));
			sr_channels = sr_device_->channels();
		}
		*/

		configurables_.push_back(make_shared<Configurable>(sr_device_));
		sr_channels = sr_device_->channel_groups()["Analog"]->channels();

		common_time_data_ = nullptr;
	}
	else {
		type_ = HardwareDevice::UNKNOWN;
		assert("Unknown device");
	}

	for (auto sr_channel : sr_channels) {
		// TODO: sr_channel is not necessarily a signal (see Digi35)....
		init_signal(sr_channel, common_time_data_, true); // TODO: fixed signal?
	}
}

HardwareDevice::~HardwareDevice()
{
	close();
}

QString HardwareDevice::full_name() const
{
	QString sep("");
	QString name("");

	if (sr_device_->vendor().length() > 0) {
		name.append(QString::fromStdString(sr_device_->vendor()));
		name.append(sep);
		sep = QString(" ");
	}

	if (sr_device_->model().length() > 0) {
		name.append(QString::fromStdString(sr_device_->model()));
		name.append(sep);
		sep = QString(" ");
	}

	if (sr_device_->version().length() > 0) {
		name.append(QString::fromStdString(sr_device_->version()));
		name.append(sep);
		sep = QString(" ");
	}

	if (sr_device_->serial_number().length() > 0) {
		name.append(QString::fromStdString(sr_device_->serial_number()));
		name.append(sep);
		sep = QString(" ");
	}

	if (sr_device_->connection_id().length() > 0) {
		name.append(sep);
		name.append("(");
		name.append(QString::fromStdString(sr_device_->connection_id()));
		name.append(")");
	}

	return name;
}

QString HardwareDevice::short_name() const
{
	QString sep("");
	QString name("");

	if (sr_device_->vendor().length() > 0) {
		name.append(QString::fromStdString(sr_device_->vendor()));
		name.append(sep);
		sep = QString(" ");
	}

	if (sr_device_->model().length() > 0) {
		name.append(QString::fromStdString(sr_device_->model()));
		name.append(sep);
		sep = QString(" ");
	}

	if (sr_device_->connection_id().length() > 0) {
		name.append(sep);
		name.append("(");
		name.append(QString::fromStdString(sr_device_->connection_id()));
		name.append(")");
	}

	return name;
}

shared_ptr<sigrok::HardwareDevice> HardwareDevice::sr_hardware_device() const
{
	return static_pointer_cast<sigrok::HardwareDevice>(sr_device_);
}

string HardwareDevice::display_name(
	const DeviceManager &device_manager) const
{
	const auto hw_dev = sr_hardware_device();

	// If we can find another device with the same model/vendor then
	// we have at least two such devices and need to distinguish them.
	const auto &devices = device_manager.devices();
	const bool multiple_dev = hw_dev && any_of(
		devices.begin(), devices.end(),
		[&](shared_ptr<devices::HardwareDevice> dev) {
			return dev->sr_hardware_device()->vendor() ==
					hw_dev->vendor() &&
				dev->sr_hardware_device()->model() ==
					hw_dev->model() &&
				dev->sr_device_ != sr_device_;
		});

	vector<string> parts = {
		sr_device_->vendor(), sr_device_->model() };

	if (multiple_dev) {
		parts.push_back(sr_device_->version());
		parts.push_back(sr_device_->serial_number());

		if ((sr_device_->serial_number().length() == 0) &&
			(sr_device_->connection_id().length() > 0))
			parts.push_back("(" + sr_device_->connection_id() + ")");
	}

	return join(parts, " ");
}

void HardwareDevice::open(function<void (const QString)> error_handler)
{
	if (device_open_)
		close();

	try {
		sr_device_->open();
	} catch (const sigrok::Error &e) {
		throw QString(e.what());
	}

	// Add device to session (do this in constructor??)
	sr_session_->add_device(sr_device_);

	sr_session_->add_datafeed_callback([=]
		(shared_ptr<sigrok::Device> sr_device, shared_ptr<sigrok::Packet> sr_packet) {
			data_feed_in(sr_device, sr_packet);
		});

	device_open_ = true;

	//stop_capture();

	// Check that at least one channel is enabled
	/*
	const auto channels = sr_device_->channels();
	if (!any_of(channels.begin(), channels.end(),
		[](shared_ptr<Channel> channel) {
			return channel->enabled(); })) {
		error_handler(tr("No channels enabled."));
		return;
	}
	*/

	// Clear signal data
	/*
	for (const shared_ptr<data::BaseData> d : all_signal_data_)
		d->clear();
	*/

	// Start aquisition
	aquisition_thread_ = std::thread(
		&HardwareDevice::aquisition_thread_proc, this, error_handler);

	aquisition_state_ = aquisition_state::Running;
}

void HardwareDevice::close()
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

shared_ptr<data::AnalogData> HardwareDevice::init_time_data()
{
	shared_ptr<data::AnalogData> time_data = make_shared<data::AnalogData>();
	time_data->set_fixed_quantity(true);
	time_data->set_quantity(sigrok::Quantity::TIME);
	time_data->set_unit(sigrok::Unit::SECOND);

	return time_data;
}

shared_ptr<data::BaseSignal> HardwareDevice::init_signal(
	shared_ptr<sigrok::Channel> sr_channel,
	shared_ptr<data::AnalogData> common_time_data,
	bool quantity_fixed)
{
	shared_ptr<data::BaseSignal> signal;
	//lock_guard<recursive_mutex> lock(data_mutex_);

	switch(sr_channel->type()->id()) {
	case SR_CHANNEL_LOGIC:
		// Not supported at the moment
		break;

	case SR_CHANNEL_ANALOG:
	{
		signal = make_shared<data::BaseSignal>(
			sr_channel, data::BaseSignal::AnalogChannel, quantity_fixed);

		qWarning() << "init_signal(): Init signal " << signal->internal_name();

		signal->set_time_start(QDateTime::currentMSecsSinceEpoch());

		if (common_time_data)
			signal->set_time_data(common_time_data);
		else {
			signal->set_time_data(init_time_data());
		}

		auto data = make_shared<data::AnalogData>();

		if (signal->internal_name().startsWith("V")) {
			data->set_fixed_quantity(true);
			data->set_quantity(sigrok::Quantity::VOLTAGE);
			data->set_unit(sigrok::Unit::VOLT);
		}
		else if (signal->internal_name().startsWith("I")) {
			data->set_fixed_quantity(true);
			data->set_quantity(sigrok::Quantity::CURRENT);
			data->set_unit(sigrok::Unit::AMPERE);
		}
		// TODO: Power
		else if (signal->internal_name().startsWith("F")) {
			data->set_fixed_quantity(true);
			data->set_quantity(sigrok::Quantity::FREQUENCY);
			data->set_unit(sigrok::Unit::HERTZ);
		}
		else if (signal->internal_name() == "P1") {
			data->set_fixed_quantity(false);
		}
		else if (signal->internal_name().startsWith("A")) {
			data->set_fixed_quantity(true);
			data->set_quantity(sigrok::Quantity::VOLTAGE);
			data->set_unit(sigrok::Unit::VOLT);
		}

		signal->set_data(data);

		all_signals_.push_back(signal);
		channel_data_.insert(pair<
			shared_ptr<sigrok::Channel>,
			shared_ptr<data::BaseSignal>>
				(sr_channel, signal));

		if (signal->internal_name().startsWith("V") && !voltage_signal_)
			voltage_signal_ = signal;
		else if (signal->internal_name().startsWith("I") && !current_signal_)
			current_signal_ = signal;
		else if (signal->internal_name() == "P1" && !measurement_signal_)
			measurement_signal_ = signal;
		else if (signal->internal_name() == "A1" && !measurement_signal_)
			measurement_signal_ = signal;

		break;
	}

	default:
		assert(false);
		break;
	}

	//signals_changed();

	return signal;
}

void HardwareDevice::aquisition_thread_proc(
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


shared_ptr<data::BaseSignal> HardwareDevice::voltage_signal() const
{
	return voltage_signal_;
}

shared_ptr<data::BaseSignal> HardwareDevice::current_signal() const
{
	return current_signal_;
}

shared_ptr<data::BaseSignal> HardwareDevice::measurement_signal() const
{
	return measurement_signal_;
}

vector<shared_ptr<data::BaseSignal>> HardwareDevice::all_signals() const
{
	return all_signals_;
}

vector<shared_ptr<devices::Configurable>> HardwareDevice::configurables() const
{
	return configurables_;
}

} // namespace devices
} // namespace sv
