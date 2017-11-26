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
#include "src/devices/device.hpp"
#include "src/data/analogdata.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/basedata.hpp"

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
	vector<shared_ptr<sigrok::Channel>> sr_channels;
	shared_ptr<data::AnalogData> common_time_data_; // TODO: Per channel group?

	const auto sr_keys = sr_device->driver()->config_keys();
	if (sr_keys.count(sigrok::ConfigKey::POWER_SUPPLY)) {
		type_ = HardwareDevice::POWER_SUPPLY;

		if (sr_device_->channel_groups().size() > 0) {
			// TODO: Handle all channel groups of a multi channel PSU
			sr_configurable_ = sr_device_->channel_groups()["1"];
			sr_channels = sr_device_->channel_groups()["1"]->channels();
		}
		else {
			sr_configurable_ = sr_device_;
			sr_channels = sr_device_->channels();
		}

		// TODO: solve this somehow with the detection of frames....
		// TODO: What if the device has multi channels with a frame around each cg data
		// PPUs DON'T have common time data
		common_time_data_ = nullptr;
	}
	else if (sr_keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD)) {
		type_ = HardwareDevice::ELECTRONIC_LOAD;

		// TODO: Handle all channel groups of a multi channel load
		sr_configurable_ = sr_device_->channel_groups()["1"];
		sr_channels = sr_device_->channel_groups()["1"]->channels();

		// TODO: solve this somehow with the detection of frames....
		// TODO: What if the device has multi channels with a frame around each cg data
		// Loads have common time data
		common_time_data_ = init_time_data();
	}
	else if (sr_keys.count(sigrok::ConfigKey::MULTIMETER)) {
		type_ = HardwareDevice::MULTIMETER;
		sr_configurable_ = sr_device_;
		sr_channels = sr_device_->channels();
		common_time_data_ = nullptr;
	}
	else {
		type_ = HardwareDevice::UNKNOWN;
	}

	for (auto sr_channel : sr_channels) {
		// TODO: sr_channel is not necessarily a signal (see Digi35)....
		init_signal(sr_channel, common_time_data_);
	}
}

HardwareDevice::~HardwareDevice()
{
	close();
}

string HardwareDevice::full_name() const
{
	vector<string> parts = {sr_device_->vendor(), sr_device_->model(),
		sr_device_->version(), sr_device_->serial_number()};
	if (sr_device_->connection_id().length() > 0)
		parts.push_back("(" + sr_device_->connection_id() + ")");
	return join(parts, " ");
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

	vector<string> parts = {sr_device_->vendor(), sr_device_->model()};

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
	shared_ptr<data::AnalogData> common_time_data)
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
			sr_channel, data::BaseSignal::AnalogChannel);

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

		signal->set_data(data);

		channel_data_.insert(pair<
			shared_ptr<sigrok::Channel>,
			shared_ptr<data::BaseSignal>>
				(sr_channel, signal));

		if (signal->internal_name().startsWith("V"))
			voltage_signal_ = signal;
		else if (signal->internal_name().startsWith("I"))
			current_signal_ = signal;
		else if (signal->internal_name() == "P1")
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


bool HardwareDevice::is_controllable() const
{
	return true;
}

bool HardwareDevice::is_enable_getable() const
{
	return is_read_config(sigrok::ConfigKey::ENABLED);
}

bool HardwareDevice::is_enable_setable() const
{
	return is_write_config(sigrok::ConfigKey::ENABLED);
}

bool HardwareDevice::get_enabled() const
{
	return read_config<bool>(sigrok::ConfigKey::ENABLED);
}

void HardwareDevice::set_enable(const bool enable)
{
	write_config(sigrok::ConfigKey::ENABLED, enable);
}


bool HardwareDevice::is_voltage_target_getable() const
{
	return is_read_config(sigrok::ConfigKey::VOLTAGE_TARGET);
}

bool HardwareDevice::is_voltage_target_setable() const
{
	return is_write_config(sigrok::ConfigKey::VOLTAGE_TARGET);
}

double HardwareDevice::get_voltage_target() const
{
	return read_config<double>(sigrok::ConfigKey::VOLTAGE_TARGET);
}

void HardwareDevice::set_voltage_target(const double value)
{
	write_config(sigrok::ConfigKey::VOLTAGE_TARGET, value);
}

void HardwareDevice::list_voltage_target(double &min, double &max, double &step)
{
	Glib::VariantContainerBase gvar = sr_configurable_->config_list(
		sigrok::ConfigKey::VOLTAGE_TARGET);

	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	min = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	max = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	step = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
}


bool HardwareDevice::is_current_limit_getable() const
{
	return is_read_config(sigrok::ConfigKey::CURRENT_LIMIT);
}

bool HardwareDevice::is_current_limit_setable() const
{
	return is_write_config(sigrok::ConfigKey::CURRENT_LIMIT);
}

double HardwareDevice::get_current_limit() const
{
	return read_config<double>(sigrok::ConfigKey::CURRENT_LIMIT);
}

void HardwareDevice::set_current_limit(const double value)
{
	write_config(sigrok::ConfigKey::CURRENT_LIMIT, value);
}

void HardwareDevice::list_current_limit(double &min, double &max, double &step)
{
	Glib::VariantContainerBase gvar = sr_configurable_->config_list(
		sigrok::ConfigKey::CURRENT_LIMIT);

	// TODO: do a better way and check!
	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	min = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	max = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	step = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
}


bool HardwareDevice::is_over_voltage_active_getable() const
{
	return is_read_config(sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE);
}

bool HardwareDevice::get_over_voltage_active() const
{
	return read_config<bool>(sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE);
}


bool HardwareDevice::is_over_current_active_getable() const
{
	return is_read_config(sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE);
}

bool HardwareDevice::get_over_current_active() const
{
	return read_config<bool>(sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE);
}


bool HardwareDevice::is_over_temperature_active_getable() const
{
	return is_read_config(sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE);
}

bool HardwareDevice::get_over_temperature_active() const
{
	return read_config<bool>(sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE);
}


bool HardwareDevice::is_under_voltage_enable_getable() const
{
	return is_read_config(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION);
}

bool HardwareDevice::is_under_voltage_enable_setable() const
{
	return is_write_config(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION);
}

bool HardwareDevice::get_under_voltage_enable() const
{
	return read_config<bool>(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION);
}


bool HardwareDevice::is_under_voltage_active_getable() const
{
	return is_read_config(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE);
}

bool HardwareDevice::get_under_voltage_active() const
{
	return read_config<bool>(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE);
}


bool HardwareDevice::is_under_voltage_threshold_getable() const
{
	return is_read_config(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
}

bool HardwareDevice::is_under_voltage_threshold_setable() const
{
	return is_write_config(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
}

double HardwareDevice::get_under_voltage_threshold() const
{
	return read_config<double>(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
}

void HardwareDevice::set_under_voltage_threshold(const double value)
{
	write_config(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD, value);
}

void HardwareDevice::list_under_voltage_threshold(
	double &min, double &max, double &step)
{
	Glib::VariantContainerBase gvar = sr_configurable_->config_list(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);

	// TODO: do a better way and check!
	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	min = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	max = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	step = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
}


} // namespace devices
} // namespace sv
