/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
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

#include "analog.hpp"
#include "analogsegment.hpp"
#include "signalbase.hpp"
#include "signaldata.hpp"
#include "src/session.hpp"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::shared_ptr;
using std::tie;
using std::unique_lock;

namespace sv {
namespace data {

const int SignalBase::ColourBGAlpha = 8 * 256 / 100;

SignalBase::SignalBase(shared_ptr<sigrok::Channel> sr_channel, ChannelType channel_type) :
	sr_channel_(sr_channel),
	channel_type_(channel_type),
	min_value_(0),
	max_value_(0)
{
	if (sr_channel_)
		internal_name_ = QString::fromStdString(sr_channel_->name());
}

SignalBase::~SignalBase()
{
}

shared_ptr<sigrok::Channel> SignalBase::sr_channel() const
{
	return sr_channel_;
}

QString SignalBase::name() const
{
	return (sr_channel_) ? QString::fromStdString(sr_channel_->name()) : name_;
}

QString SignalBase::internal_name() const
{
	return internal_name_;
}

void SignalBase::set_name(QString name)
{
	if (sr_channel_)
		sr_channel_->set_name(name.toUtf8().constData());

	name_ = name;

	name_changed(name);
}

bool SignalBase::enabled() const
{
	return (sr_channel_) ? sr_channel_->enabled() : true;
}

void SignalBase::set_enabled(bool value)
{
	if (sr_channel_) {
		sr_channel_->set_enabled(value);
		enabled_changed(value);
	}
}

SignalBase::ChannelType SignalBase::type() const
{
	return channel_type_;
}

unsigned int SignalBase::index() const
{
	return (sr_channel_) ? sr_channel_->index() : 0;
}

QColor SignalBase::colour() const
{
	return colour_;
}

void SignalBase::set_colour(QColor colour)
{
	colour_ = colour;

	bgcolour_ = colour;
	bgcolour_.setAlpha(ColourBGAlpha);

	colour_changed(colour);
}

QColor SignalBase::bgcolour() const
{
	return bgcolour_;
}

void SignalBase::set_data(shared_ptr<sv::data::SignalData> data)
{
	if (data_) {
		disconnect(data.get(), SIGNAL(samples_cleared()),
			this, SLOT(on_samples_cleared()));
		disconnect(data.get(), SIGNAL(samples_added(QObject*, uint64_t, uint64_t)),
			this, SLOT(on_samples_added(QObject*, uint64_t, uint64_t)));

		if (channel_type_ == AnalogChannel) {
			shared_ptr<Analog> analog = analog_data();
			assert(analog);

			disconnect(analog.get(), SIGNAL(min_max_changed(float, float)),
				this, SLOT(on_min_max_changed(float, float)));
		}
	}

	data_ = data;

	if (data_) {
		connect(data.get(), SIGNAL(samples_cleared()),
			this, SLOT(on_samples_cleared()));
		connect(data.get(), SIGNAL(samples_added(QObject*, uint64_t, uint64_t)),
			this, SLOT(on_samples_added(QObject*, uint64_t, uint64_t)));

		if (channel_type_ == AnalogChannel) {
			shared_ptr<Analog> analog = analog_data();
			assert(analog);

			connect(analog.get(), SIGNAL(min_max_changed(float, float)),
				this, SLOT(on_min_max_changed(float, float)));
		}
	}
}

shared_ptr<data::Analog> SignalBase::analog_data() const
{
	shared_ptr<Analog> result = nullptr;

	if (channel_type_ == AnalogChannel)
		result = dynamic_pointer_cast<Analog>(data_);

	return result;
}

void SignalBase::save_settings(QSettings &settings) const
{
	settings.setValue("name", name());
	settings.setValue("enabled", enabled());
	settings.setValue("colour", colour());
}

void SignalBase::restore_settings(QSettings &settings)
{
	set_name(settings.value("name").toString());
	set_enabled(settings.value("enabled").toBool());
	set_colour(settings.value("colour").value<QColor>());
}

void SignalBase::on_samples_cleared()
{
	samples_cleared();
}

void SignalBase::on_samples_added(QObject* segment, uint64_t start_sample,
	uint64_t end_sample)
{
	samples_added(segment, start_sample, end_sample);
}

void SignalBase::on_min_max_changed(float min, float max)
{
	min_max_changed(min, max);
}

void SignalBase::on_capture_state_changed(int state)
{
	if (state == Session::Running) {
	}
}

} // namespace data
} // namespace sv
