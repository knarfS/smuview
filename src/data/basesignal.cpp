/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
#include <string>

#include <QDebug>
#include <QString>

#include "basesignal.hpp"
#include "src/data/datautil.hpp"

using std::set;
using std::string;

namespace sv {
namespace data {

BaseSignal::BaseSignal(
		data::Quantity quantity,
		const set<data::QuantityFlag> &quantity_flags,
		data::Unit unit,
		const string &custom_name,
		const string &parent_device_id,
		const string &parent_device_name,
		const set<string> &parent_channel_group_names,
		const string &parent_channel_name) :
	quantity_(quantity),
	quantity_flags_(quantity_flags),
	unit_(unit),
	parent_device_id_(parent_device_id),
	parent_device_name_(parent_device_name),
	parent_channel_group_names_(parent_channel_group_names),
	parent_channel_name_(parent_channel_name)
{
	/* TODO
	if (!util::is_valid_sr_quantity(sr_quantity_))
		throw std::runtime_error("Invalide quantity for BaseSignal()");
	*/

	quantity_name_ = data::datautil::format_quantity(quantity_);
	quantity_flags_name_ = data::datautil::format_quantity_flags(
		quantity_flags_, QString(" "));
	unit_name_ = data::datautil::format_unit(unit_);

	if (custom_name.empty()) {
		name_ = parent_channel_name_ + " [" + unit_name_.toStdString();
		if (!quantity_flags_.empty())
			name_ += " " + quantity_flags_name_.toStdString();
		name_ += "]";
	}
	else {
		name_ = custom_name;
	}
}

BaseSignal::~BaseSignal()
{
	//qWarning() << "BaseSignal::~BaseSignal(): " << display_name();
}

data::Quantity BaseSignal::quantity() const
{
	return quantity_;
}

QString BaseSignal::quantity_name() const
{
	return quantity_name_;
}

set<data::QuantityFlag> BaseSignal::quantity_flags() const
{
	return quantity_flags_;
}

QString BaseSignal::quantity_flags_name() const
{
	return quantity_flags_name_;
}

data::Unit BaseSignal::unit() const
{
	return unit_;
}

QString BaseSignal::unit_name() const
{
	return unit_name_;
}

void BaseSignal::set_name(const string &custom_name)
{
	if (!custom_name.empty()) {
		name_ = custom_name;
		Q_EMIT name_changed(name_);
	}
}

string BaseSignal::name() const
{
	return name_;
}

QString BaseSignal::display_name() const
{
	return QString::fromStdString(name_);
}

string BaseSignal::parent_device_id() const
{
	return parent_device_id_;
}

string BaseSignal::parent_device_name() const
{
	return parent_device_name_;
}

set<string> BaseSignal::parent_channel_group_names() const
{
	return parent_channel_group_names_;
}

string BaseSignal::parent_channel_name() const
{
	return parent_channel_name_;
}

} // namespace data
} // namespace sv
