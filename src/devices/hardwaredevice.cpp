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
#include <boost/algorithm/string/join.hpp>

#include <QDateTime>
#include <QDebug>
#include <QString>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "hardwaredevice.hpp"
#include "src/devicemanager.hpp"
#include "src/session.hpp"
#include "src/data/analog.hpp"
#include "src/data/signalbase.hpp"
#include "src/data/signaldata.hpp"

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
	sr_context_(sr_context),
	device_open_(false)
{
	// TODO: sigrok::Device and not sigrok::HardwareDevice in constructor?? then cast...
	sr_device_ = sr_device;

	const auto sr_keys = sr_device->driver()->config_keys();
	if (sr_keys.count(sigrok::ConfigKey::POWER_SUPPLY))
		type_ = HardwareDevice::POWER_SUPPLY;
	else if (sr_keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD))
		type_ = HardwareDevice::ELECTRONIC_LOAD;
	else if (sr_keys.count(sigrok::ConfigKey::MULTIMETER))
		type_ = HardwareDevice::MULTIMETER;
	else
		type_ = HardwareDevice::UNKNOWN;

	vector<shared_ptr<sigrok::Channel>> sr_channels;
	if (type_ == POWER_SUPPLY) {
		// TODO: Handle all channel groups of a multi channel PSU
		if (sr_device_->channel_groups().size() > 0) {
			sr_configurable_ = sr_device_->channel_groups()["1"];
			sr_channels = sr_device_->channel_groups()["1"]->channels();
		}
		else {
			sr_configurable_ = sr_device_;
			sr_channels = sr_device_->channels();
		}

		// TODO: solve this somehow with the detection of frames....
		// TODO: What if the device has multi channels with a frame around each cg data
		// PPUs have common time data
		common_time_data_ = make_shared<data::Analog>();
	}
	else if (type_ == ELECTRONIC_LOAD) {
		sr_configurable_ = sr_device_->channel_groups()["1"];
		sr_channels = sr_device_->channel_groups()["1"]->channels();

		// TODO: solve this somehow with the detection of frames....
		// TODO: What if the device has multi channels with a frame around each cg data
		// Loads have common time data
		common_time_data_ = make_shared<data::Analog>();
	}

	for (auto sr_channel : sr_channels) {
		init_signal(sr_channel);
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

void HardwareDevice::open()
{
	if (device_open_)
		close();

	try {
		sr_device_->open();
	} catch (const sigrok::Error &e) {
		throw QString(e.what());
	}

	device_open_ = true;
}

void HardwareDevice::close()
{
	if (device_open_)
		sr_device_->close();

	// TODO
	//if (session_)
	//	session_->remove_devices();

	device_open_ = false;
}

shared_ptr<data::SignalBase> HardwareDevice::init_signal(
	shared_ptr<sigrok::Channel> sr_channel)
{
	qWarning() << "init_signal() -1-";
	shared_ptr<data::SignalBase> signalbase;
	//lock_guard<recursive_mutex> lock(data_mutex_);

	switch(sr_channel->type()->id()) {
	case SR_CHANNEL_LOGIC:
		// Not supported at the moment
		break;

	case SR_CHANNEL_ANALOG:
	{
		signalbase = make_shared<data::SignalBase>(
			sr_channel, data::SignalBase::AnalogChannel);

		signalbase->set_time_start(QDateTime::currentMSecsSinceEpoch());

		if (common_time_data_)
			signalbase->set_time_data(common_time_data_);
		else {
			shared_ptr<data::Analog> time_data = make_shared<data::Analog>();
			signalbase->set_time_data(time_data);
		}

		shared_ptr<data::Analog> data = make_shared<data::Analog>();
		signalbase->set_data(data);

		channel_data_.insert(pair<
			shared_ptr<sigrok::Channel>,
			shared_ptr<data::SignalBase>>
				(sr_channel, signalbase));

		if (signalbase->internal_name().startsWith("V"))
			voltage_signal_ = signalbase;
		else if (signalbase->internal_name().startsWith("I"))
			current_signal_ = signalbase;
		else if (signalbase->internal_name() == "P1")
			measurement_signal_ = signalbase;

		break;
	}

	default:
		assert(false);
		break;
	}

	//signals_changed();

	return signalbase;
}

shared_ptr<data::SignalBase> HardwareDevice::voltage_signal() const
{
	return voltage_signal_;
}

shared_ptr<data::SignalBase> HardwareDevice::current_signal() const
{
	return current_signal_;
}

shared_ptr<data::SignalBase> HardwareDevice::measurement_signal() const
{
	return measurement_signal_;
}


bool HardwareDevice::is_controllable() const
{
	return true;
}

bool HardwareDevice::is_enable_getable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::ENABLED, sigrok::Capability::GET);
}

bool HardwareDevice::is_enable_setable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::ENABLED, sigrok::Capability::SET);
}

bool HardwareDevice::get_enabled() const
{
	// TODO: check if getable

	bool enable;
	try {
		auto gvar = sr_configurable_->config_get(sigrok::ConfigKey::ENABLED);
		enable =
			Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(gvar).get();
	} catch (sigrok::Error error) {
		qDebug() << "Failed to get ENABLED.";
		return false;
	}

	return enable;
}

void HardwareDevice::set_enable(const bool enable)
{
	// TODO: check if setable

	try {
		sr_configurable_->config_set(
			sigrok::ConfigKey::ENABLED, Glib::Variant<bool>::create(enable));
	} catch (sigrok::Error error) {
		qDebug() << "Failed to set ENABLED.";
	}
}


bool HardwareDevice::is_voltage_target_getable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::VOLTAGE_TARGET, sigrok::Capability::GET);
}

bool HardwareDevice::is_voltage_target_setable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::VOLTAGE_TARGET, sigrok::Capability::SET);
}

double HardwareDevice::get_voltage_target() const
{
	// TODO: check if getable

	double value;
	try {
		auto gvar = sr_configurable_->config_get(
			sigrok::ConfigKey::VOLTAGE_TARGET);
		value =
			Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	} catch (sigrok::Error error) {
		qDebug() << "Failed to get VOLTAGE_TARGET.";
		return false;
	}

	return value;
}

void HardwareDevice::set_voltage_target(const double value)
{
	if (!device_open_)
		return;

	sr_configurable_->config_set(sigrok::ConfigKey::VOLTAGE_TARGET,
		Glib::Variant<double>::create(value));
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
	return sr_configurable_->config_check(
		sigrok::ConfigKey::CURRENT_LIMIT, sigrok::Capability::GET);
}

bool HardwareDevice::is_current_limit_setable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::CURRENT_LIMIT, sigrok::Capability::SET);
}

double HardwareDevice::get_current_limit() const
{
	// TODO: check if getable

	double value;
	try {
		auto gvar = sr_configurable_->config_get(
			sigrok::ConfigKey::CURRENT_LIMIT);
		value =
			Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	} catch (sigrok::Error error) {
		qDebug() << "Failed to get CURRENT_LIMIT.";
		return false;
	}

	return value;
}

void HardwareDevice::set_current_limit(const double value)
{
	if (!device_open_)
		return;

	sr_configurable_->config_set(sigrok::ConfigKey::CURRENT_LIMIT,
		Glib::Variant<double>::create(value));
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
	return sr_configurable_->config_check(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE,
		sigrok::Capability::GET);
}

double HardwareDevice::get_over_voltage_active() const
{
	// TODO: check if getable

	bool active;
	try {
		auto gvar = sr_configurable_->config_get(
			sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE);
		active =
			Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(gvar).get();
	} catch (sigrok::Error error) {
		qDebug() << "Failed to get OVER_VOLTAGE_PROTECTION_ACTIVE.";
		return false;
	}

	return active;
}


bool HardwareDevice::is_over_current_active_getable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE,
		sigrok::Capability::GET);
}

double HardwareDevice::get_over_current_active() const
{
	// TODO: check if getable

	bool active;
	try {
		auto gvar = sr_configurable_->config_get(
			sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE);
		active =
			Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(gvar).get();
	} catch (sigrok::Error error) {
		qDebug() << "Failed to get OVER_CURRENT_PROTECTION_ACTIVE.";
		return false;
	}

	return active;
}


bool HardwareDevice::is_over_temperature_active_getable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE,
		sigrok::Capability::GET);
}

double HardwareDevice::get_over_temperature_active() const
{
	// TODO: check if getable

	bool active;
	try {
		auto gvar = sr_configurable_->config_get(
			sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE);
		active =
			Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(gvar).get();
	} catch (sigrok::Error error) {
		qDebug() << "Failed to get OVER_TEMPERATURE_PROTECTION_ACTIVE.";
		return false;
	}

	return active;
}


bool HardwareDevice::is_under_voltage_enable_getable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION,
		sigrok::Capability::GET);
}

bool HardwareDevice::is_under_voltage_enable_setable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION,
		sigrok::Capability::SET);
}

double HardwareDevice::get_under_voltage_enable() const
{
	// TODO: check if getable

	bool enable;
	try {
		auto gvar = sr_configurable_->config_get(
			sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION);
		enable =
			Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(gvar).get();
	} catch (sigrok::Error error) {
		qDebug() << "Failed to get UNDER_VOLTAGE_CONDITION.";
		return false;
	}

	return enable;
}


bool HardwareDevice::is_under_voltage_active_getable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE,
		sigrok::Capability::GET);
}

double HardwareDevice::get_under_voltage_active() const
{
	// TODO: check if getable

	bool active;
	try {
		auto gvar = sr_configurable_->config_get(
			sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE);
		active =
			Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(gvar).get();
	} catch (sigrok::Error error) {
		qDebug() << "Failed to get UNDER_VOLTAGE_CONDITION_ACTIVE.";
		return false;
	}

	return active;
}


bool HardwareDevice::is_under_voltage_threshold_getable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD,
		sigrok::Capability::GET);
}

bool HardwareDevice::is_under_voltage_threshold_setable() const
{
	return sr_configurable_->config_check(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD,
		sigrok::Capability::SET);
}

double HardwareDevice::get_under_voltage_threshold() const
{
	// TODO: check if getable

	double value;
	try {
		auto gvar = sr_configurable_->config_get(
			sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
		value =
			Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	} catch (sigrok::Error error) {
		qDebug() << "Failed to get UNDER_VOLTAGE_CONDITION_THRESHOLD.";
		return false;
	}

	return value;
}

void HardwareDevice::set_under_voltage_threshold(const double value)
{
	if (!device_open_)
		return;

	sr_configurable_->config_set(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD,
		Glib::Variant<double>::create(value));
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
