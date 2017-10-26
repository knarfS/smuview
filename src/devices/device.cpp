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

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>

#include "device.hpp"
#include "src/data/analog.hpp"
#include "src/data/analogsegment.hpp"
#include "src/data/segment.hpp"
#include "src/data/signalbase.hpp"
#include "src/data/signaldata.hpp"

using std::bad_alloc;
using std::dynamic_pointer_cast;
using std::lock_guard;
using std::make_shared;
using std::set;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::vector;
using std::unique_ptr;

using std::shared_ptr;

using sigrok::ConfigKey;
using sigrok::Capability;

using Glib::VariantBase;
using Glib::Variant;

namespace sv {
namespace devices {

Device::~Device()
{
	//if (session_)
	//	session_->remove_datafeed_callbacks();

	// TODO:
	//remove_datafeed_callbacks();
}

shared_ptr<sigrok::Device> Device::sr_device() const
{
	return sr_device_;
}

template
uint64_t Device::read_config(const sigrok::ConfigKey*,
	const uint64_t);

template<typename T>
T Device::read_config(const ConfigKey *key, const T default_value)
{
	assert(key);

	if (!sr_device_)
		return default_value;

	if (!sr_device_->config_check(key, Capability::GET))
		return default_value;

	return VariantBase::cast_dynamic<Glib::Variant<guint64>>(
		sr_device_->config_get(ConfigKey::SAMPLERATE)).get();
}

shared_ptr<data::SignalBase> Device::signalbase_from_channel(
	shared_ptr<sigrok::Channel> sr_channel) const
{
	for (shared_ptr<data::SignalBase> sig : signalbases_) {
		assert(sig);
		if (sig->sr_channel() == sr_channel) {
			return sig;
		}
	}
	return shared_ptr<data::SignalBase>();
}

void Device::feed_in_header()
{
}

void Device::feed_in_meta(shared_ptr<sigrok::Meta> meta)
{
	for (auto entry : meta->config()) {
		switch (entry.first->id()) {
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
		case SR_CONF_OVER_VOLTAGE_PROTECTION_ACTIVE:
			Q_EMIT over_voltage_protection_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_CURRENT_PROTECTION_ACTIVE:
			Q_EMIT over_current_protection_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_UNDER_VOLTAGE_CONDITION_ACTIVE:
			Q_EMIT under_voltage_condition_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		case SR_CONF_OVER_TEMPERATURE_PROTECTION_ACTIVE:
			Q_EMIT over_temperature_protection_active_changed(
				g_variant_get_boolean(entry.second.gobj()));
			break;
		default:
			// Unknown metadata is not an error.
			break;
		}
	}

	//Q_EMIT signals_changed();
}

void Device::feed_in_trigger()
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

void Device::feed_in_frame_begin()
{
	frame_began_ = true;

	//if (!cur_analog_segments_.empty())
		//Q_EMIT frame_began();
}

void Device::feed_in_frame_end()
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

void Device::feed_in_analog(shared_ptr<sigrok::Analog> sr_analog)
{
	lock_guard<recursive_mutex> lock(data_mutex_);

	const vector<shared_ptr<sigrok::Channel>> sr_channels = sr_analog->channels();
	const unsigned int channel_count = sr_channels.size();
	const size_t sample_count = sr_analog->num_samples() / channel_count;
	bool sweep_beginning = false;

	unique_ptr<float> data(new float[sr_analog->num_samples()]);
	sr_analog->get_data_as_float(data.get());

	if (signalbases_.empty())
		update_signals();

	float *channel_data = data.get();
	qWarning("feed_in_analog() data: %f", *channel_data);
	for (auto sr_channel : sr_channels) {
		qWarning() << "Channel.Id: " << QString::fromStdString(sr_channel->name());
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

		//Q_EMIT data_received(segment);
	}

	if (sweep_beginning) {
		// This could be the first packet after a trigger
		//set_capture_state(Session::Running);
	}
}

void Device::data_feed_in(shared_ptr<sigrok::Device> sr_device,
	shared_ptr<sigrok::Packet> sr_packet)
{
	(void)sr_device;

	assert(sr_device);
	assert(sr_packet);

	if (sr_device != sr_device_)
		return;

	switch (sr_packet->type()->id()) {
	case SR_DF_HEADER:
		qWarning() << "data_feed_in(): SR_DF_HEADER";
		feed_in_header();
		break;

	case SR_DF_META:
		qWarning() << "data_feed_in(): SR_DF_META";
		feed_in_meta(
			dynamic_pointer_cast<sigrok::Meta>(sr_packet->payload()));
		break;

	case SR_DF_TRIGGER:
		qWarning() << "data_feed_in(): SR_DF_TRIGGER";
		feed_in_trigger();
		break;

	case SR_DF_LOGIC:
		qWarning() << "data_feed_in(): SR_DF_LOGIC";
		break;

	case SR_DF_ANALOG:
		qWarning() << "data_feed_in(): SR_DF_ANALOG";
		try {
			feed_in_analog(
				dynamic_pointer_cast<sigrok::Analog>(sr_packet->payload()));
		} catch (bad_alloc) {
			out_of_memory_ = true;
			// TODO: sr_session->stop();
		}
		break;

	case SR_DF_FRAME_BEGIN:
		qWarning() << "data_feed_in(): SR_DF_FRAME_BEGIN";
		feed_in_frame_begin();
		break;

	case SR_DF_FRAME_END:
		qWarning() << "data_feed_in(): SR_DF_FRAME_END";
		feed_in_frame_end();
		break;

	case SR_DF_END:
		qWarning() << "data_feed_in(): SR_DF_END";
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

void Device::update_signals()
{
	qWarning() << "update_signals() -1-";
	lock_guard<recursive_mutex> lock(data_mutex_);
	qWarning() << "update_signals() -2-";

	if (!sr_device_) {
		signalbases_.clear();
		/*
		for (shared_ptr<views::ViewBase> view : views_) {
			view->clear_signals();
		}
		*/
		return;
	}

	// Detect what data types we will receive
	auto sr_channels = sr_device_->channels();
	unsigned int analog_channel_count = count_if(
		sr_channels.begin(), sr_channels.end(),
		[] (shared_ptr<sigrok::Channel> sr_channel) {
			return sr_channel->type() == sigrok::ChannelType::ANALOG; });

	// Create data containers for the logic data segments
	{
		lock_guard<recursive_mutex> data_lock(data_mutex_);

		if (analog_channel_count == 0) {
			analog_data_.reset();
		} else if (!analog_data_ /*||
			analog_data_->num_channels() != analog_channel_count*/) {
			/*
			analog_data_.reset(new data::Analog(
				analog_channel_count));*/
			analog_data_ = make_shared<data::Analog>();
			assert(analog_data_);
		}
	}

	// Make the signals list
	/*
	for (shared_ptr<views::ViewBase> viewbase : views_) {
		views::trace::View *trace_view =
			qobject_cast<views::trace::View*>(viewbase.get());

		if (trace_view) {
			unordered_set< shared_ptr<views::trace::Signal> >
				prev_sigs(trace_view->signals());
			trace_view->clear_signals();
			*/

			for (auto sr_channel : sr_device_->channels()) {
				shared_ptr<data::SignalBase> signalbase;
				//shared_ptr<views::trace::Signal> signal;

				// Find the channel in the old signals
				/*
				const auto iter = find_if(
					prev_sigs.cbegin(), prev_sigs.cend(),
					[&](const shared_ptr<views::trace::Signal> &s) {
						return s->base()->channel() == channel;
					});
				if (iter != prev_sigs.end()) {
					// Copy the signal from the old set to the new
					signal = *iter;
					trace_view->add_signal(signal);
				} else {
					*/
					// Find the signalbase for this channel if possible
					signalbase.reset();
					for (const shared_ptr<data::SignalBase> b : signalbases_)
						if (b->sr_channel() == sr_channel)
							signalbase = b;

					switch(sr_channel->type()->id()) {
					case SR_CHANNEL_LOGIC:
						/*
						if (!signalbase) {
							signalbase = make_shared<data::SignalBase>(channel,
								data::SignalBase::LogicChannel);
							signalbases_.insert(signalbase);

							all_signal_data_.insert(logic_data_);
							signalbase->set_data(logic_data_);

							connect(this, SIGNAL(capture_state_changed(int)),
								signalbase.get(), SLOT(on_capture_state_changed(int)));
						}

						signal = shared_ptr<views::trace::Signal>(
							new views::trace::LogicSignal(*this,
								device_, signalbase));
						trace_view->add_signal(signal);
						*/
						break;

					case SR_CHANNEL_ANALOG:
					{
						if (!signalbase) {
							signalbase = make_shared<data::SignalBase>(
								sr_channel, data::SignalBase::AnalogChannel);
							signalbases_.insert(signalbase);

							shared_ptr<data::Analog> data(new data::Analog());
							all_signal_data_.insert(data);
							signalbase->set_data(data);

							/*
							connect(this, SIGNAL(capture_state_changed(int)),
								signalbase.get(), SLOT(on_capture_state_changed(int)));
								*/
						}

						/*
						signal = shared_ptr<views::trace::Signal>(
							new views::trace::AnalogSignal(
								*this, signalbase));
						trace_view->add_signal(signal);
						*/
						break;
					}

					default:
						assert(false);
						break;
					}
					/*
				}
				*/
			}
			/*
		}
	}
	*/

	//signals_changed();
}

} // namespace devices
} // namespace sv
