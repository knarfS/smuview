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

#ifndef DATA_BASESIGNAL_HPP
#define DATA_BASESIGNAL_HPP

#include <atomic>
#include <condition_variable>
#include <thread>
#include <vector>

#include <QColor>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QVariant>

using std::atomic;
using std::condition_variable;
using std::map;
using std::mutex;
using std::pair;
using std::shared_ptr;
using std::vector;

namespace sigrok {
class Channel;
class Quantity;
class QuantityFlag;
class Unit;
}

namespace sv {
namespace data {

class BaseSignal : public QObject
{
	Q_OBJECT

public:
	BaseSignal(
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit,
		QString channel_name, QString channel_group_name);
	virtual ~BaseSignal();

public:
	virtual void clear() = 0;
	virtual size_t get_sample_count() const = 0;

	/**
	 * Add a single sample to the signal
	 */
	virtual void push_sample(void *sample,
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit) = 0;

	/**
	 * Add a single sample with a timestampto the signal
	 */
	virtual void push_sample(void *sample, double timestamp,
		const sigrok::Quantity *sr_quantity,
		vector<const sigrok::QuantityFlag *> sr_quantity_flags,
		const sigrok::Unit *sr_unit) = 0;

	/**
	 * Returns the sigrok qunatity of this signal as object
	 */
	const sigrok::Quantity *sr_quantity() const;

	/**
	 * Returns the qunatity of this signal as string
	 */
	QString quantity() const;

	/**
	 * Returns the sigrok qunatity flags of this signal as vector
	 */
	vector<const sigrok::QuantityFlag *> sr_quantity_flags() const;

	/**
	 * Returns the qunatity flags of this signal as string
	 */
	QString quantity_flags() const;

	/**
	 * Returns the sigrok unit of this signal as object
	 */
	const sigrok::Unit *sr_unit() const;

	/**
	 * Returns the unit of this signal as string
	 */
	QString unit() const;

	/**
	 * Gets the name of this signal.
	 */
	QString name() const;

	/**
	 * Get the colour of the signal.
	 */
	QColor colour() const;

	/**
	 * Set the colour of the signal.
	 */
	void set_colour(QColor colour);

protected:
	const sigrok::Quantity *sr_quantity_;
	QString quantity_;
	vector<const sigrok::QuantityFlag *> sr_quantity_flags_;
	QString quantity_flags_;
	const sigrok::Unit *sr_unit_;
	QString unit_;
	const QString channel_name_;
	const QString channel_group_name_;

	QString name_; // TODO: const?
	QColor colour_;

Q_SIGNALS:
	void colour_changed(const QColor &colour);

};

} // namespace data
} // namespace sv

#endif // DATA_BASESIGNAL_HPP
