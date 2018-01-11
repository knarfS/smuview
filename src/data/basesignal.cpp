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
#include <utility>

#include <QDateTime>
#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "basesignal.hpp"
#include "src/session.hpp"
#include "src/util.hpp"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::pair;
using std::shared_ptr;

namespace sv {
namespace data {

BaseSignal::BaseSignal(
		shared_ptr<sigrok::Channel> sr_channel, ChannelType channel_type,
		const sigrok::Quantity *sr_quantity) :
	sr_channel_(sr_channel),
	channel_type_(channel_type),
	sr_quantity_(sr_quantity)
{
	internal_name_ = QString::fromStdString(sr_channel_->name());
	quantity_ = util::format_sr_quantity(sr_quantity_);
	sr_unit_ = util::get_sr_unit_from_sr_quantity(sr_quantity_);
	unit_ = util::format_sr_unit(sr_unit_);
}

BaseSignal::~BaseSignal()
{
}

shared_ptr<sigrok::Channel> BaseSignal::sr_channel() const
{
	return sr_channel_;
}

QString BaseSignal::quantity() const
{
	return quantity_;
}

QString BaseSignal::unit() const
{
	return unit_;
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

} // namespace data
} // namespace sv
