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
#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "channel.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/data/analogsignal.hpp"

using std::make_pair;
using std::make_shared;

namespace sv {
namespace devices {

Channel::Channel(
		shared_ptr<sigrok::Channel> sr_channel, ChannelType channel_type,
		QString channel_group_name, double channel_start_timestamp) :
	sr_channel_(sr_channel),
	channel_type_(channel_type),
	channel_start_timestamp_(channel_start_timestamp),
	actual_signal_(nullptr),
	channel_group_name_(channel_group_name)
{
	internal_name_ = QString::fromStdString(sr_channel_->name());
	name_ = internal_name_;
	qWarning() << "Init channel " << internal_name_ << ", channel_start_timestamp = " << channel_start_timestamp;
}

Channel::~Channel()
{
}

shared_ptr<sigrok::Channel> Channel::sr_channel() const
{
	return sr_channel_;
}

bool Channel::has_fixed_signal()
{
	return has_fixed_signal_;
}

void Channel::set_fixed_signal(bool has_fixed_signal)
{
	has_fixed_signal_ = has_fixed_signal;
}

shared_ptr<data::BaseSignal> Channel::actual_signal()
{
	return actual_signal_;
}

map<Channel::quantity_t, shared_ptr<data::BaseSignal>> Channel::signal_map()
{
	return signal_map_;
}

QString Channel::channel_group_name() const
{
	return channel_group_name_;
}

QString Channel::name() const
{
	return name_;
}

QString Channel::internal_name() const
{
	return internal_name_;
}

void Channel::set_name(QString name)
{
	if (sr_channel_)
		sr_channel_->set_name(name.toUtf8().constData());

	name_ = name;

	name_changed(name);
}

bool Channel::enabled() const
{
	return (sr_channel_) ? sr_channel_->enabled() : true;
}

void Channel::set_enabled(bool value)
{
	if (sr_channel_) {
		sr_channel_->set_enabled(value);
		enabled_changed(value);
	}
}

Channel::ChannelType Channel::type() const
{
	return channel_type_;
}

unsigned int Channel::index() const
{
	return (sr_channel_) ? sr_channel_->index() : 0;
}

QColor Channel::colour() const
{
	return colour_;
}

void Channel::set_colour(QColor colour)
{
	colour_ = colour;
	colour_changed(colour);
}

shared_ptr<data::BaseSignal> Channel::init_signal(
	const sigrok::Quantity *sr_quantity,
	vector<const sigrok::QuantityFlag *> sr_quantity_flags,
	const sigrok::Unit *sr_unit)
{
	// TODO: At the moment, only analog channels are supported
	if (sr_channel_->type()->id() != SR_CHANNEL_ANALOG)
		return nullptr;

	shared_ptr<data::AnalogSignal> signal = make_shared<data::AnalogSignal>(
		sr_quantity, sr_quantity_flags, sr_unit,
		internal_name_, channel_group_name_, channel_start_timestamp_);

	connect(this, SIGNAL(channel_start_timestamp_changed(double)),
			signal.get(), SLOT(on_channel_start_timestamp_changed(double)));

	actual_signal_ = signal;
	quantity_t q_qf = make_pair(sr_quantity, sr_quantity_flags);
	signal_map_.insert(
		pair<quantity_t, shared_ptr<data::BaseSignal>>
		(q_qf, signal));

	return signal;
}

void Channel::push_sample(void *sample,
	const sigrok::Quantity *sr_quantity,
	vector<const sigrok::QuantityFlag *> sr_quantity_flags,
	const sigrok::Unit *sr_unit)
{
	// TODO: use std::chrono / std::time
	double timestamp = QDateTime::currentMSecsSinceEpoch() / (double)1000;
	push_sample(sample, timestamp, sr_quantity, sr_quantity_flags, sr_unit);
}

void Channel::push_sample(void *sample, double timestamp,
	const sigrok::Quantity *sr_quantity,
	vector<const sigrok::QuantityFlag *> sr_quantity_flags,
	const sigrok::Unit *sr_unit)
{
	quantity_t q_qf = make_pair(sr_quantity, sr_quantity_flags);
	if (signal_map_.count(q_qf) == 0) {
		init_signal(sr_quantity, sr_quantity_flags, sr_unit);
		Q_EMIT signal_changed();
		qWarning() << "Channel::push_sample(): " << internal_name_ <<
			" - No signal found: " << actual_signal_->name();
	}

	signal_map_[q_qf]->push_sample(
		sample, timestamp, sr_quantity, sr_quantity_flags, sr_unit);
}

void Channel::save_settings(QSettings &settings) const
{
	settings.setValue("name", name());
	settings.setValue("enabled", enabled());
	settings.setValue("colour", colour());
}

void Channel::restore_settings(QSettings &settings)
{
	set_name(settings.value("name").toString());
	set_enabled(settings.value("enabled").toBool());
	set_colour(settings.value("colour").value<QColor>());
}

void Channel::on_aquisition_start_timestamp_changed(double timestamp)
{
	channel_start_timestamp_ = timestamp;
	Q_EMIT channel_start_timestamp_changed(timestamp);
}

} // namespace devices
} // namespace sv
