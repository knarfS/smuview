/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_BASESIGNAL_HPP
#define DATA_BASESIGNAL_HPP

#include <memory>
#include <set>
#include <string>

#include <QColor>
#include <QObject>
#include <QString>

#include "src/data/datautil.hpp"

using std::set;
using std::shared_ptr;
using std::string;

namespace sv {

namespace channels {
class BaseChannel;
}

namespace data {

class BaseSignal : public QObject
{
	Q_OBJECT

public:
	BaseSignal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		shared_ptr<channels::BaseChannel> parent_channel);
	virtual ~BaseSignal();

public:
	/**
	 * Clear all samples from this signal.
	 */
	virtual void clear() = 0;

	/**
	 * Return the number of samples in this signal.
	 */
	virtual size_t sample_count() const = 0;

	/**
	 * Return the qunatity of this signal.
	 */
	data::Quantity quantity() const;

	/**
	 * Return the qunatity of this signal as string
	 */
	QString quantity_name() const;

	/**
	 * Return the quantity flags of this signal as set
	 */
	set<data::QuantityFlag> quantity_flags() const;

	/**
	 * Return the qunatity flags of this signal as string
	 */
	QString quantity_flags_name() const;

	/**
	 * Return the unit of this signal
	 */
	data::Unit unit() const;

	/**
	 * Return the unit of this signal as string
	 */
	QString unit_name() const;

	/**
	 * Return the parent channel, this signal belongs to
	 */
	shared_ptr<channels::BaseChannel> parent_channel() const;

	/**
	 * Get the name of this signal.
	 */
	string name() const;

	/**
	 * Get the display name of this signal.
	 */
	QString display_name() const;

protected:
	data::Quantity quantity_;
	QString quantity_name_;
	set<data::QuantityFlag> quantity_flags_;
	QString quantity_flags_name_;
	data::Unit unit_;
	QString unit_name_;
	shared_ptr<channels::BaseChannel> parent_channel_;

	string name_;

};

} // namespace data
} // namespace sv

#endif // DATA_BASESIGNAL_HPP
