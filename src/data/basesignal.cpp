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
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit,
		const QString device_name, const QString channel_group_name,
		const QString channel_name) :
	sr_quantity_(sr_quantity),
	sr_quantity_flags_(sr_quantity_flags),
	sr_unit_(sr_unit),
	device_name_(device_name),
	channel_group_name_(channel_group_name),
	channel_name_(channel_name)
{
	if (!util::is_valid_sr_quantity(sr_quantity_))
		assert("Invalide quantity for BaseSignal()");

	quantity_ = util::format_sr_quantity(sr_quantity_);
	quantity_flags_ = util::format_sr_quantity_flags(sr_quantity_flags_);
	unit_ = util::format_sr_unit(sr_unit_);

	name_ = QString("%1 [%2").arg(channel_name, quantity_);
	if (quantity_flags_.size() > 0)
		name_ = name_.append(" ").append(quantity_flags_);
	name_ = name_.append("]");
}

BaseSignal::~BaseSignal()
{
}

const sigrok::Quantity *BaseSignal::sr_quantity() const
{
	return sr_quantity_;
}

QString BaseSignal::quantity() const
{
	return quantity_;
}

vector<const sigrok::QuantityFlag *> BaseSignal::sr_quantity_flags() const
{
	return sr_quantity_flags_;
}

QString BaseSignal::quantity_flags() const
{
	return quantity_flags_;
}

const sigrok::Unit *BaseSignal::sr_unit() const
{
	return sr_unit_;
}

QString BaseSignal::unit() const
{
	return unit_;
}

QString BaseSignal::device_name() const
{
	return device_name_;
}

QString BaseSignal::channel_group_name() const
{
	return channel_group_name_;
}

QString BaseSignal::channel_name() const
{
	return channel_name_;
}

QString BaseSignal::name() const
{
	return name_;
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

} // namespace data
} // namespace sv
