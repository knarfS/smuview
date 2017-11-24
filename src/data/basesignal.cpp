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

#include <assert.h>

#include <QDateTime>

#include "basesignal.hpp"
#include "src/session.hpp"
#include "src/data/analogdata.hpp"
#include "src/data/basedata.hpp"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::shared_ptr;
using std::tie;
using std::unique_lock;

namespace sv {
namespace data {

BaseSignal::BaseSignal(shared_ptr<sigrok::Channel> sr_channel,
		ChannelType channel_type) :
	sr_channel_(sr_channel),
	channel_type_(channel_type)
{
	if (sr_channel_)
		internal_name_ = QString::fromStdString(sr_channel_->name());
}

BaseSignal::~BaseSignal()
{
}

shared_ptr<sigrok::Channel> BaseSignal::sr_channel() const
{
	return sr_channel_;
}

QString BaseSignal::name() const
{
	return (sr_channel_) ? QString::fromStdString(sr_channel_->name()) : name_;
}

QString BaseSignal::internal_name() const
{
	return internal_name_;
}

void BaseSignal::set_name(QString name)
{
	if (sr_channel_)
		sr_channel_->set_name(name.toUtf8().constData());

	name_ = name;

	name_changed(name);
}

bool BaseSignal::enabled() const
{
	return (sr_channel_) ? sr_channel_->enabled() : true;
}

void BaseSignal::set_enabled(bool value)
{
	if (sr_channel_) {
		sr_channel_->set_enabled(value);
		enabled_changed(value);
	}
}

BaseSignal::ChannelType BaseSignal::type() const
{
	return channel_type_;
}

unsigned int BaseSignal::index() const
{
	return (sr_channel_) ? sr_channel_->index() : 0;
}

QColor BaseSignal::colour() const
{
	return colour_;
}

void BaseSignal::set_colour(QColor colour)
{
	colour_ = colour;
	colour_changed(colour);
}

void BaseSignal::set_time_start(qint64 time_start)
{
	time_start_ = time_start;
}

void BaseSignal::set_data(shared_ptr<sv::data::BaseData> data)
{
	/*
	if (data_) {
		disconnect(data.get(), SIGNAL(samples_cleared()),
			this, SLOT(on_samples_cleared()));
		disconnect(data.get(), SIGNAL(samples_added(QObject*, uint64_t, uint64_t)),
			this, SLOT(on_samples_added(QObject*, uint64_t, uint64_t)));
	}
	*/

	data_ = data;

	/*
	if (data_) {
		connect(data.get(), SIGNAL(samples_cleared()),
			this, SLOT(on_samples_cleared()));
		connect(data.get(), SIGNAL(samples_added(QObject*, uint64_t, uint64_t)),
			this, SLOT(on_samples_added(QObject*, uint64_t, uint64_t)));
	}
	*/
}

void BaseSignal::set_time_data(shared_ptr<sv::data::AnalogData> time_data)
{
	time_data_ = time_data;
}

void BaseSignal::add_timestamp()
{
	// TODO: use std::chrono / std::time and double
	qint64 time_span = QDateTime::currentMSecsSinceEpoch() - time_start_;
	float dtime_span = time_span / (float)1000;
	time_data_->push_sample(&dtime_span);
}

shared_ptr<sv::data::BaseData> BaseSignal::data()
{
	return data_;
}

shared_ptr<data::AnalogData> BaseSignal::analog_data() const
{
	shared_ptr<AnalogData> result = nullptr;

	if (channel_type_ == AnalogChannel)
		result = dynamic_pointer_cast<AnalogData>(data_);

	return result;
}

shared_ptr<data::AnalogData> BaseSignal::time_data() const
{
	return time_data_;
}

void BaseSignal::save_settings(QSettings &settings) const
{
	settings.setValue("name", name());
	settings.setValue("enabled", enabled());
	settings.setValue("colour", colour());
}

void BaseSignal::restore_settings(QSettings &settings)
{
	set_name(settings.value("name").toString());
	set_enabled(settings.value("enabled").toBool());
	set_colour(settings.value("colour").value<QColor>());
}

void BaseSignal::on_samples_cleared()
{
	samples_cleared();
}

} // namespace data
} // namespace sv
