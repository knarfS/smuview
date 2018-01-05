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
#include "src/data/analogdata.hpp"
#include "src/data/basedata.hpp"
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

using sigrok::ConfigKey;
using sigrok::Capability;

namespace sv {
namespace devices {

Device::Device(const shared_ptr<sigrok::Context> &sr_context):
	sr_context_(sr_context)
{
	init_device();
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

void Device::init_device()
{
	// Set up the session
	sr_session_ = sv::Session::sr_context->create_session();
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

void Device::feed_in_header()
{
}

void Device::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	for (auto entry : sr_meta->config()) {
		switch (entry.first->id()) {
		/*
		case SR_CONF_ENABLED:
			Q_EMIT enabled_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_VOLTAGE_TARGET:
			Q_EMIT voltage_target_changed(
				g_variant_get_double(entry.second.gobj()));
			break;
		case SR_CONF_CURRENT_LIMIT:
			Q_EMIT current_limit_changed(
				g_variant_get_double(entry.second.gobj()));
			break;

		case SR_CONF_OVER_TEMPERATURE_PROTECTION:
			Q_EMIT otp_enabled_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_TEMPERATURE_PROTECTION_ACTIVE:
			Q_EMIT otp_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;

		case SR_CONF_OVER_VOLTAGE_PROTECTION_ENABLED:
			Q_EMIT ovp_enabled_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_VOLTAGE_PROTECTION_ACTIVE:
			Q_EMIT ovp_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_VOLTAGE_PROTECTION_THRESHOLD:
			Q_EMIT ovp_threshold_changed(
				g_variant_get_double(entry.second.gobj()));
			break;

		case SR_CONF_OVER_CURRENT_PROTECTION_ENABLED:
			Q_EMIT ocp_enabled_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_CURRENT_PROTECTION_ACTIVE:
			Q_EMIT ocp_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_CURRENT_PROTECTION_THRESHOLD:
			Q_EMIT ocp_threshold_changed(
				g_variant_get_double(entry.second.gobj()));
			break;

		case SR_CONF_UNDER_VOLTAGE_CONDITION:
			Q_EMIT uvc_enabled_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_UNDER_VOLTAGE_CONDITION_ACTIVE:
			Q_EMIT uvc_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		*/
		/*
		case SR_CONF_UNDER_VOLTAGE_CONDITION_THRESHOLD:
			Q_EMIT uvc_threshold_changed(
				g_variant_get_double(entry.second.gobj()));
			break;
		*/
		default:
			// Unknown metadata is not an error.
			break;
		}
	}

	//Q_EMIT signals_changed();
}

void Device::feed_in_trigger()
{
}

void Device::feed_in_frame_begin()
{
	frame_began_ = true;
}

void Device::feed_in_frame_end()
{
	if (frame_began_ && actual_processed_signal_) {
		/*
		qWarning() << "feed_in_frame_end(): Set timestamp to " <<
			actual_processed_signal_->name();
		*/
		// TODO: This may not work reliably
		actual_processed_signal_->add_timestamp();
		actual_processed_signal_ = nullptr;

	}
	frame_began_ = false;
}

void Device::feed_in_analog(shared_ptr<sigrok::Analog> sr_analog)
{
	lock_guard<recursive_mutex> lock(data_mutex_);

	const vector<shared_ptr<sigrok::Channel>> sr_channels = sr_analog->channels();
	//const unsigned int channel_count = sr_channels.size();
	//const size_t sample_count = sr_analog->num_samples() / channel_count;

	unique_ptr<float> data(new float[sr_analog->num_samples()]);
	sr_analog->get_data_as_float(data.get());

	float *channel_data = data.get();
	for (auto sr_channel : sr_channels) {
		/*
		qWarning() << "Device::feed_in_analog(): Device = " <<
			QString::fromStdString(sr_device_->model()) <<
			", Channel.Id = " <<
			QString::fromStdString(sr_channel->name()) <<
			" channel_data = " << *channel_data;
		*/

		if (!sr_channel_signal_map_.count(sr_channel)) {
			/*
			qWarning() << "Device::feed_in_analog(): Channel " <<
				QString::fromStdString(sr_channel->name()) <<
				" not found, adding";
			*/

			// TODO: notwendig? Better handling for new channels? Are there new channels??
			init_signal(sr_channel, nullptr, false);
			continue;
		}

		actual_processed_signal_ = sr_channel_signal_map_[sr_channel];
		/*
		qWarning() << "Device::feed_in_analog(): -3- name = " << actual_processed_signal_->name();
		qWarning() << "Device::feed_in_analog(): -3- count = " << actual_processed_signal_->analog_data()->get_sample_count();
		*/

		/*
		actual_processed_signal_->analog_data()->push_interleaved_samples(
			channel_data, sample_count,channel_count,
			sr_analog->mq(), sr_analog->unit());
		*/
		actual_processed_signal_->analog_data()->push_sample(channel_data,
			sr_analog->mq(), sr_analog->unit());
		channel_data++;

		// Timestamp for values not in a FRAME
		if (!frame_began_)
			actual_processed_signal_->add_timestamp();

		/*
		Q_EMIT data_received(segment);
		*/
	}

	/*
	qWarning() << "Device::feed_in_analog(): -END-";
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

} // namespace devices
} // namespace sv
