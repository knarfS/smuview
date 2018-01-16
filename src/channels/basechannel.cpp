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

#include <cassert>

#include <QDateTime>
#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "basechannel.hpp"
#include "src/session.hpp"
#include "src/util.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"

using std::make_pair;
using std::make_shared;

namespace sv {
namespace channels {

BaseChannel::BaseChannel(
		const QString device_name,
		const QString channel_group_name,
		double channel_start_timestamp) :
	channel_start_timestamp_(channel_start_timestamp),
	has_fixed_signal_(false),
	actual_signal_(nullptr),
	device_name_(device_name),
	channel_group_name_(channel_group_name),
	name_("")
{
	qWarning() << "Init channel " << name_
		<< ", channel_start_timestamp = "
		<< util::format_time_date(channel_start_timestamp);
}

BaseChannel::~BaseChannel()
{
}

bool BaseChannel::has_fixed_signal()
{
	return has_fixed_signal_;
}

void BaseChannel::set_fixed_signal(bool has_fixed_signal)
{
	has_fixed_signal_ = has_fixed_signal;
}

shared_ptr<data::BaseSignal> BaseChannel::actual_signal()
{
	return actual_signal_;
}

map<BaseChannel::quantity_t, shared_ptr<data::BaseSignal>> BaseChannel::signal_map()
{
	return signal_map_;
}

QString BaseChannel::channel_group_name() const
{
	return channel_group_name_;
}

QString BaseChannel::name() const
{
	return name_;
}

void BaseChannel::set_name(QString name)
{
	name_ = name;
	name_changed(name);
}

QString BaseChannel::display_name() const
{
	return name_;
}

bool BaseChannel::enabled() const
{
	return true;
}

void BaseChannel::set_enabled(bool value)
{
	(void)value;
}

ChannelType BaseChannel::type() const
{
	return channel_type_;
}

QColor BaseChannel::colour() const
{
	return colour_;
}

void BaseChannel::set_colour(QColor colour)
{
	colour_ = colour;
	colour_changed(colour);
}

void BaseChannel::push_sample(void *sample,
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit)
{
	// TODO: use std::chrono / std::time
	double timestamp = QDateTime::currentMSecsSinceEpoch() / (double)1000;
	push_sample(sample, timestamp, sr_quantity, sr_quantity_flags, sr_unit);
}

void BaseChannel::push_sample(void *sample, double timestamp,
	const sigrok::Quantity *sr_quantity,
	vector<const sigrok::QuantityFlag *> sr_quantity_flags,
	const sigrok::Unit *sr_unit)
{
	quantity_t q_qf = make_pair(sr_quantity, sr_quantity_flags);
	if (signal_map_.count(q_qf) == 0) {
		init_signal(sr_quantity, sr_quantity_flags, sr_unit);
		Q_EMIT signal_changed();
		qWarning() << "Channel::push_sample(): " << name_ <<
		" - No signal found: " << actual_signal_->name();
	}

	signal_map_[q_qf]->push_sample(
		sample, timestamp, sr_quantity, sr_quantity_flags, sr_unit);
}

void BaseChannel::save_settings(QSettings &settings) const
{
	settings.setValue("name", name());
	settings.setValue("enabled", enabled());
	settings.setValue("colour", colour());
}

void BaseChannel::restore_settings(QSettings &settings)
{
	set_name(settings.value("name").toString());
	set_enabled(settings.value("enabled").toBool());
	set_colour(settings.value("colour").value<QColor>());
}

void BaseChannel::on_aquisition_start_timestamp_changed(double timestamp)
{
	qWarning()
		<< "BaseChannel::on_aquisition_start_timestamp_changed() timestamp = "
		<< util::format_time_date(timestamp);
	channel_start_timestamp_ = timestamp;
	Q_EMIT channel_start_timestamp_changed(timestamp);
}

} // namespace channels
} // namespace sv
