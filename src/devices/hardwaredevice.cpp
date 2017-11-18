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

HardwareDevice::HardwareDevice(const shared_ptr<sigrok::Context> &sr_context,
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
	}
	else if (type_ == ELECTRONIC_LOAD) {
		sr_configurable_ = sr_device_->channel_groups()["1"];
		sr_channels = sr_device_->channel_groups()["1"]->channels();
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

shared_ptr<data::SignalBase> HardwareDevice::voltage_signal() const
{
	for (auto const &iter : channel_data_) {
		if (iter.second->internal_name().startsWith("V"))
			return iter.second;
	}

	return Q_NULLPTR;
}

shared_ptr<data::SignalBase> HardwareDevice::current_signal() const
{
	for (auto const &iter : channel_data_) {
		if (iter.second->internal_name().startsWith("I"))
			return iter.second;
	}

	return Q_NULLPTR;
}

shared_ptr<data::SignalBase> HardwareDevice::measurement_signal() const
{
	for (auto const &iter : channel_data_) {
		if (iter.second->internal_name() == "P1")
			return iter.second;
	}

	return Q_NULLPTR;
}

shared_ptr<data::Analog> HardwareDevice::time_data() const
{
	return time_data_;
}

void HardwareDevice::get_all_config_check()
{
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
		auto gvar =
			sr_configurable_->config_get(sigrok::ConfigKey::VOLTAGE_TARGET);
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
	Glib::VariantContainerBase gvar =
		sr_configurable_->config_list(sigrok::ConfigKey::VOLTAGE_TARGET);

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
		auto gvar =
			sr_configurable_->config_get(sigrok::ConfigKey::CURRENT_LIMIT);
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
	Glib::VariantContainerBase gvar =
		sr_configurable_->config_list(sigrok::ConfigKey::CURRENT_LIMIT);

	// TODO: do a better way and check!
	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	min = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	max = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	step = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
}

/*
shared_ptr<data::SignalBase> HardwareDevice::signalbase_from_channel(
	shared_ptr<sigrok::Channel> sr_channel) const
{
	for (shared_ptr<data::SignalBase> sig : signalbases_) {
		assert(sig);
		if (sig->channel() == sr_channel)
			return sig;
	}
	return shared_ptr<data::SignalBase>();
}

void HardwareDevice::feed_in_header()
{
}

void HardwareDevice::feed_in_meta(shared_ptr<sigrok::Meta> meta)
{
	for (auto entry : meta->config()) {
		switch (entry.first->id()) {
			break;
		default:
			// Unknown metadata is not an error.
			break;
		}
	}

	//Q_EMIT signals_changed();
}

void HardwareDevice::feed_in_trigger()
{
	// The channel containing most samples should be most accurate
	uint64_t sample_count = 0;

	{
		for (const shared_ptr<data::SignalData> d : all_signal_data_) {
			assert(d);
			uint64_t temp_count = 0;

			const vector< shared_ptr<data::Segment> > segments =
				d->segments();
			for (const shared_ptr<data::Segment> &s : segments)
				temp_count += s->get_sample_count();

			if (temp_count > sample_count)
				sample_count = temp_count;
		}
	}

	//trigger_event(sample_count / get_samplerate());
}

void HardwareDevice::feed_in_frame_begin()
{
	frame_began_ = true;

	//if (!cur_analog_segments_.empty())
		//Q_EMIT frame_began();
}

void HardwareDevice::feed_in_frame_end()
{
	{
		lock_guard<recursive_mutex> lock(data_mutex_);
		cur_analog_segments_.clear();
	}

	if (frame_began_) {
		frame_began_ = false;
		//Q_EMIT frame_ended();
	}
}

void HardwareDevice::feed_in_analog(shared_ptr<sigrok::Analog> sr_analog)
{
	lock_guard<recursive_mutex> lock(data_mutex_);

	const vector<shared_ptr<sigrok::Channel>> sr_channels = sr_analog->channels();
	const unsigned int channel_count = sr_channels.size();
	const size_t sample_count = sr_analog->num_samples() / channel_count;
	bool sweep_beginning = false;

	unique_ptr<float> data(new float[sr_analog->num_samples()]);
	sr_analog->get_data_as_float(data.get());

	//if (signalbases_.empty())
	//	update_signals();

	float *channel_data = data.get();
	for (auto sr_channel : sr_channels) {
		shared_ptr<data::AnalogSegment> segment;

		// Try to get the segment of the channel
		const map< shared_ptr<sigrok::Channel>, shared_ptr<data::AnalogSegment> >::
			iterator iter = cur_analog_segments_.find(sr_channel);
		if (iter != cur_analog_segments_.end())
			segment = (*iter).second;
		else {
			// If no segment was found, this means we haven't
			// created one yet. i.e. this is the first packet
			// in the sweep containing this segment.
			sweep_beginning = true;

			// Find the analog data associated with the channel
			shared_ptr<data::SignalBase> base = signalbase_from_channel(sr_channel);
			assert(base);

			shared_ptr<data::Analog> data(base->analog_data());
			assert(data);

			// Create a segment, keep it in the maps of channels
			segment = make_shared<data::AnalogSegment>(
				*data, cur_samplerate_);
			cur_analog_segments_[sr_channel] = segment;

			// Push the segment into the analog data.
			data->push_segment(segment);
		}

		assert(segment);

		// Append the samples in the segment
		segment->append_interleaved_samples(channel_data++, sample_count,
			channel_count);
	}

	if (sweep_beginning) {
		// This could be the first packet after a trigger
		//set_capture_state(Session::Running);
	}

	//Q_EMIT data_received();
}

void HardwareDevice::data_feed_in(shared_ptr<sigrok::Device> sr_device,
	shared_ptr<sigrok::Packet> sr_packet)
{
	(void)sr_device;

	assert(sr_device);
	assert(sr_device == sr_device_);
	assert(sr_packet);

	switch (sr_packet->type()->id()) {
	case SR_DF_HEADER:
		feed_in_header();
		break;

	case SR_DF_META:
		feed_in_meta(
			dynamic_pointer_cast<sigrok::Meta>(sr_packet->payload()));
		break;

	case SR_DF_TRIGGER:
		feed_in_trigger();
		break;

	case SR_DF_LOGIC:
		break;

	case SR_DF_ANALOG:
		try {
			feed_in_analog(
				dynamic_pointer_cast<sigrok::Analog>(sr_packet->payload()));
		} catch (bad_alloc) {
			out_of_memory_ = true;
			stop();
		}
		break;

	case SR_DF_FRAME_BEGIN:
		feed_in_frame_begin();
		break;

	case SR_DF_FRAME_END:
		feed_in_frame_end();
		break;

	case SR_DF_END:
		// Strictly speaking, this is performed when a frame end marker was
		// received, so there's no point doing this again. However, not all
		// devices use frames, and for those devices, we need to do it here.
		{
			lock_guard<recursive_mutex> lock(data_mutex_);
			cur_analog_segments_.clear();
		}
		break;

	default:
		break;
	}
}
*/

} // namespace devices
} // namespace sv
