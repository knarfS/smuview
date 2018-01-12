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

using std::make_shared;

namespace sv {
namespace devices {

Channel::Channel(
		shared_ptr<sigrok::Channel> sr_channel, ChannelType channel_type,
		QString channel_group_name) :
	sr_channel_(sr_channel),
	channel_type_(channel_type),
	channel_group_name_(channel_group_name)
{
	internal_name_ = QString::fromStdString(sr_channel_->name());
}

Channel::~Channel()
{
}

shared_ptr<sigrok::Channel> Channel::sr_channel() const
{
	return sr_channel_;
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

/*
void Channel::push_sample(void *sample,
	const sigrok::Quantity *sr_quantity,
	const sigrok::QuantityFlag *sr_quantity_flag,
	const sigrok::Unit *sr_unit)
{
	// TODO: use std::chrono / std::time
	double timestamp = QDateTime::currentMSecsSinceEpoch() / (double)1000;
	push_sample(sample, timestamp, sr_quantity, sr_quantity_flag, sr_unit);
}

void Channel::push_sample(void *sample, double timestamp,
	const sigrok::Quantity *sr_quantity,
	const sigrok::QuantityFlag *sr_quantity_flag,
	const sigrok::Unit *sr_unit)
{
}
*/

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

} // namespace devices
} // namespace sv
