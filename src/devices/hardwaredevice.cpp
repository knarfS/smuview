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

	init_device_properties();
	init_device_values();

	for (auto sr_channel : sr_channels) {
		// TODO: sr_channel is not necessarily a signal (see Digi35)....
		init_signal(sr_channel, common_time_data_);
	}
}

HardwareDevice::~HardwareDevice()
{
	close();
}

void HardwareDevice::init_device_properties()
{
	is_enabled_getable_ = has_get_config(sigrok::ConfigKey::ENABLED);
	is_enabled_setable_ = has_set_config(sigrok::ConfigKey::ENABLED);

	is_regulation_getable_ = has_get_config(sigrok::ConfigKey::REGULATION);
	is_regulation_setable_ = has_set_config(sigrok::ConfigKey::REGULATION);
	is_regulation_listable_ = has_list_config(sigrok::ConfigKey::REGULATION);

	is_voltage_target_getable_ = has_get_config(
		sigrok::ConfigKey::VOLTAGE_TARGET);
	is_voltage_target_setable_ = has_set_config(
		sigrok::ConfigKey::VOLTAGE_TARGET);
	is_voltage_target_listable_ = has_list_config(
		sigrok::ConfigKey::VOLTAGE_TARGET);
	is_current_limit_getable_ = has_get_config(
		sigrok::ConfigKey::CURRENT_LIMIT);
	is_current_limit_setable_ = has_set_config(
		sigrok::ConfigKey::CURRENT_LIMIT);
	is_current_limit_listable_ = has_list_config(
		sigrok::ConfigKey::CURRENT_LIMIT);

	is_otp_enabled_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION);
	is_otp_enabled_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION);
	is_otp_active_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE);
	is_otp_active_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE);

	is_ovp_enabled_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED);
	is_ovp_enabled_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED);
	is_ovp_active_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE);
	is_ovp_active_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE);
	is_ovp_threshold_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD);
	is_ovp_threshold_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD);
	is_ovp_threshold_listable_ = has_list_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD);

	is_ocp_enabled_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED);
	is_ocp_enabled_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED);
	is_ocp_active_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE);
	is_ocp_active_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE);
	is_ocp_threshold_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD);
	is_ocp_threshold_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD);
	is_ocp_threshold_listable_ = has_list_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD);

	is_uvc_enabled_getable_ = has_get_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION);
	is_uvc_enabled_setable_ = has_set_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION);
	is_uvc_active_getable_ = has_get_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE);
	is_uvc_active_setable_ = has_set_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE);
	is_uvc_threshold_getable_ = has_get_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
	is_uvc_threshold_setable_ = has_set_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
	is_uvc_threshold_listable_ = has_list_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);

	is_measured_quantity_getable_ = has_get_config(
		sigrok::ConfigKey::MEASURED_QUANTITY);
	is_measured_quantity_setable_ = has_set_config(
		sigrok::ConfigKey::MEASURED_QUANTITY);
	is_measured_quantity_listable_ = has_list_config(
		sigrok::ConfigKey::MEASURED_QUANTITY);
}

void HardwareDevice::init_device_values()
{
	if (is_regulation_listable_)
		list_config_string_array(sigrok::ConfigKey::REGULATION,
			regulation_list_);

	if (is_voltage_target_listable_)
		list_config_min_max_steps(sigrok::ConfigKey::VOLTAGE_TARGET,
			voltage_target_min_, voltage_target_max_, voltage_target_step_);

	if (is_current_limit_listable_)
		list_config_min_max_steps(sigrok::ConfigKey::CURRENT_LIMIT,
			current_limit_min_, current_limit_max_, current_limit_step_);

	if (is_ovp_threshold_listable_)
		list_config_min_max_steps(
			sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD,
			ovp_threshold_min_, ovp_threshold_max_, ovp_threshold_step_);

	if (is_ocp_threshold_listable_)
		list_config_min_max_steps(
			sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD,
			ocp_threshold_min_, ocp_threshold_max_, ocp_threshold_step_);

	if (is_uvc_threshold_listable_)
		list_config_min_max_steps(
			sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD,
			uvc_threshold_min_, uvc_threshold_max_, uvc_threshold_step_);

	if (is_measured_quantity_listable_)
		list_config_mq(sigrok::ConfigKey::MEASURED_QUANTITY,
			sr_mq_flags_list_, mq_flags_list_);
}

string HardwareDevice::full_name() const
{
	vector<string> parts = {
		sr_device_->vendor(), sr_device_->model(),
		sr_device_->version(), sr_device_->serial_number() };
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


bool HardwareDevice::get_enabled() const
{
	return get_config<bool>(sigrok::ConfigKey::ENABLED);
}

void HardwareDevice::set_enabled(const bool enabled)
{
	set_config(sigrok::ConfigKey::ENABLED, enabled);
}


bool HardwareDevice::get_regulation() const
{
	return get_config<bool>(sigrok::ConfigKey::REGULATION);
}

void HardwareDevice::set_regulation(const bool regulation)
{
	set_config(sigrok::ConfigKey::REGULATION, regulation);
}


double HardwareDevice::get_voltage_target() const
{
	return get_config<double>(sigrok::ConfigKey::VOLTAGE_TARGET);
}

void HardwareDevice::set_voltage_target(const double value)
{
	set_config(sigrok::ConfigKey::VOLTAGE_TARGET, value);
}


double HardwareDevice::get_current_limit() const
{
	return get_config<double>(sigrok::ConfigKey::CURRENT_LIMIT);
}

void HardwareDevice::set_current_limit(const double value)
{
	set_config(sigrok::ConfigKey::CURRENT_LIMIT, value);
}


bool HardwareDevice::get_ovp_enabled() const
{
	return get_config<bool>(sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED);
}

void HardwareDevice::set_ovp_enabled(const bool enabled)
{
	set_config(sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED, enabled);
}

bool HardwareDevice::get_ovp_active() const
{
	if (is_ovp_active_getable_)
		return get_config<bool>(
			sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE);
	else
		return false;
}

double HardwareDevice::get_ovp_threshold() const
{
	return get_config<double>(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD);
}

void HardwareDevice::set_ovp_threshold(const double threshold)
{
	set_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD, threshold);
}


bool HardwareDevice::get_ocp_enabled() const
{
	return get_config<bool>(sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED);
}

void HardwareDevice::set_ocp_enabled(const bool enabled)
{
	set_config(sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED, enabled);
}

bool HardwareDevice::get_ocp_active() const
{
	if (is_ocp_active_getable_)
		return get_config<bool>(
			sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE);
	else
		return false;
}

double HardwareDevice::get_ocp_threshold() const
{
	return get_config<double>(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD);
}

void HardwareDevice::set_ocp_threshold(const double threshold)
{
	set_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD, threshold);
}


bool HardwareDevice::get_otp_active() const
{
	if (is_otp_active_getable_)
		return get_config<bool>(
			sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE);
	else
		return false;
}


bool HardwareDevice::get_uvc_enabled() const
{
	return get_config<bool>(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION);
}

void HardwareDevice::set_uvc_enabled(const bool enabled)
{
	set_config(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION, enabled);
}

bool HardwareDevice::get_uvc_active() const
{
	if (is_uvc_active_getable_)
		return get_config<bool>(
			sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE);
	else
		return false;
}

double HardwareDevice::get_uvc_threshold() const
{
	return get_config<double>(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
}

void HardwareDevice::set_uvc_threshold(const double threshold)
{
	set_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD, threshold);
}


void HardwareDevice::get_measured_quantity() const
{
	//get_config<double>(sigrok::ConfigKey::MEASURED_QUANTITY);
}

void HardwareDevice::set_measured_quantity(uint mq, uint mq_flags)
{
	mq = mq;
	mq_flags = mq_flags;

	//set_config(sigrok::ConfigKey::MEASURED_QUANTITY, mq);
}


bool HardwareDevice::list_regulation(QStringList &regulation_list)
{
	if (!is_regulation_listable_)
		return false;

	regulation_list = regulation_list_;
	return true;
}

bool HardwareDevice::list_voltage_target(double &min, double &max, double &step)
{
	if (!is_voltage_target_listable_)
		return false;

	min = voltage_target_min_;
	max = voltage_target_max_;
	step = voltage_target_step_;
	return true;
}

bool HardwareDevice::list_current_limit(double &min, double &max, double &step)
{
	if (!is_current_limit_listable_)
		return false;

	min = current_limit_min_;
	max = current_limit_max_;
	step = current_limit_step_;
	return true;
}

bool HardwareDevice::list_ovp_threshold(double &min, double &max, double &step)
{
	if (!is_ovp_threshold_listable_)
		return false;

	min = ovp_threshold_min_;
	max = ovp_threshold_max_;
	step = ovp_threshold_step_;
	return true;
}

bool HardwareDevice::list_ocp_threshold(double &min, double &max, double &step)
{
	if (!is_ocp_threshold_listable_)
		return false;

	min = ocp_threshold_min_;
	max = ocp_threshold_max_;
	step = ocp_threshold_step_;
	return true;
}

bool HardwareDevice::list_uvc_threshold(double &min, double &max, double &step)
{
	if (!is_uvc_threshold_listable_)
		return false;

	min = uvc_threshold_min_;
	max = uvc_threshold_max_;
	step = uvc_threshold_step_;
	return true;
}

bool HardwareDevice::list_measured_quantity(
	sr_mq_flags_list_t &sr_mq_flags_list, mq_flags_list_t &mq_flags_list)
{
	if (!is_measured_quantity_listable_)
		return false;

	sr_mq_flags_list = sr_mq_flags_list_;
	mq_flags_list = mq_flags_list_;
	return true;
}


bool HardwareDevice::is_controllable() const
{
	if (type_ == Type::POWER_SUPPLY || type_ == Type::ELECTRONIC_LOAD) {
		if (is_enabled_setable_ || is_regulation_setable_ ||
			is_voltage_target_setable_ || is_current_limit_setable_ ||
			is_ovp_enabled_setable_ || is_ovp_threshold_setable_ ||
			is_ocp_enabled_setable_ || is_ocp_threshold_setable_ ||
			is_uvc_enabled_setable_ || is_uvc_threshold_setable_)

			return true;
	}
	else if (type_ == Type::MULTIMETER) {
		if (is_measured_quantity_getable_ || is_measured_quantity_setable_)
			return true;
	}

	return false;
}


bool HardwareDevice::is_enabled_getable() const
{
	return is_enabled_getable_;
}

bool HardwareDevice::is_enabled_setable() const
{
	return is_enabled_setable_;
}


bool HardwareDevice::is_regulation_getable() const
{
	return is_regulation_getable_;
}

bool HardwareDevice::is_regulation_setable() const
{
	return is_regulation_setable_;
}


bool HardwareDevice::is_voltage_target_getable() const
{
	return is_voltage_target_getable_;
}

bool HardwareDevice::is_voltage_target_setable() const
{
	return is_voltage_target_setable_;
}

bool HardwareDevice::is_voltage_target_listable() const
{
	return is_voltage_target_listable_;
}


bool HardwareDevice::is_current_limit_getable() const
{
	return is_current_limit_getable_;
}

bool HardwareDevice::is_current_limit_setable() const
{
	return is_current_limit_setable_;
}

bool HardwareDevice::is_current_limit_listable() const
{
	return is_current_limit_listable_;
}


bool HardwareDevice::is_ovp_enabled_getable() const
{
	return is_ovp_enabled_getable_;
}

bool HardwareDevice::is_ovp_enabled_setable() const
{
	return is_ovp_enabled_setable_;
}

bool HardwareDevice::is_ovp_active_getable() const
{
	return is_ovp_active_getable_;
}

bool HardwareDevice::is_ovp_threshold_getable() const
{
	return is_ovp_threshold_getable_;
}

bool HardwareDevice::is_ovp_threshold_setable() const
{
	return is_ovp_threshold_setable_;
}

bool HardwareDevice::is_ovp_threshold_listable() const
{
	return is_ovp_threshold_listable_;
}


bool HardwareDevice::is_ocp_enabled_getable() const
{
	return is_ocp_enabled_getable_;
}

bool HardwareDevice::is_ocp_enabled_setable() const
{
	return is_ocp_enabled_setable_;
}

bool HardwareDevice::is_ocp_active_getable() const
{
	return is_ocp_active_getable_;
}

bool HardwareDevice::is_ocp_threshold_getable() const
{
	return is_ocp_threshold_getable_;
}

bool HardwareDevice::is_ocp_threshold_setable() const
{
	return is_ocp_threshold_setable_;
}

bool HardwareDevice::is_ocp_threshold_listable() const
{
	return is_ocp_threshold_listable_;
}


bool HardwareDevice::is_otp_active_getable() const
{
	return is_otp_active_getable_;
}

bool HardwareDevice::is_uvc_enabled_getable() const
{
	return is_uvc_enabled_getable_;
}

bool HardwareDevice::is_uvc_enabled_setable() const
{
	return is_uvc_enabled_setable_;
}

bool HardwareDevice::is_uvc_active_getable() const
{
	return is_uvc_active_getable_;
}

bool HardwareDevice::is_uvc_threshold_getable() const
{
	return is_uvc_threshold_getable_;
}

bool HardwareDevice::is_uvc_threshold_setable() const
{
	return is_uvc_threshold_setable_;
}

bool HardwareDevice::is_uvc_threshold_listable() const
{
	return is_uvc_threshold_listable_;
}


bool HardwareDevice::is_measured_quantity_getable() const
{
	return is_measured_quantity_getable_;
}

bool HardwareDevice::is_measured_quantity_setable() const
{
	return is_measured_quantity_setable_;
}

bool HardwareDevice::is_measured_quantity_listable() const
{
	return is_measured_quantity_listable_;
}


} // namespace devices
} // namespace sv
