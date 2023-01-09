/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2016-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef CHANNELS_BASECHANNEL_HPP
#define CHANNELS_BASECHANNEL_HPP

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <QObject>
#include <QSettings>
#include <QString>

#include "src/data/datautil.hpp"

using std::map;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;
using sv::data::measured_quantity_t;

namespace sigrok {
class Channel;
}

namespace sv {

namespace data {
class AnalogTimeSignal;
class BaseSignal;
}

namespace devices {
class BaseDevice;
}

namespace channels {

enum class ChannelType {
	/**
	 * Channels with analog data (Power supplies, loads, DMMs)
	 */
	AnalogChannel,
	/**
	 * Virtual channel for calculated data
	 */
	MathChannel,
	/**
	 * Virtual channel for user generated data (e.g. scripts)
	 */
	UserChannel
};

class BaseChannel :
	public QObject,
	public std::enable_shared_from_this<BaseChannel>
{
	Q_OBJECT

public:
	BaseChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		const set<string> &channel_group_names,
		double channel_start_timestamp);
	virtual ~BaseChannel();

public:
	/**
	 * Return the underlying sigrok channel.
	 *
	 * HardwareChannels always have a sigrok channel, UserChannels only have a
	 * sigrok channel when created in a UserDevice!
	 */
	shared_ptr<sigrok::Channel> sr_channel() const;

	/**
	 * Get the name of this channel, i.e. how the device calls it.
	 */
	string name() const;

	/**
	 * Set the name of the signal.
	 */
	void set_name(const string &name);

	/**
	 * Get the display name of this channel.
	 */
	QString display_name() const;

	/**
	 * Get the index number of this channel, i.e. a unique ID assigned by
	 * the device driver.
	 */
	unsigned int index() const;

	/**
	 * Get the type of this channel.
	 */
	ChannelType type() const;

	/**
	 * Return enabled status of this channel.
	 */
	bool enabled() const;

	/**
	 * Set the enabled status of this channel.
	 */
	void set_enabled(bool enabled);

	/**
	 * Does this channel have just one signal, thats quantity doesn't change?
	 */
	bool fixed_signal() const;

	/**
	 * Set if this channel has just one signal, thats quantity doesn't change
	 */
	void set_fixed_signal(bool fixed_signal);

	/**
	 * Return the device, this channel belongs to.
	 */
	shared_ptr<devices::BaseDevice> parent_device();

	/**
	 * Get the channel group name, the channel is in. Returns "" if the channel
	 * is not in a channel group.
	 */
	set<string> channel_group_names() const;

	/**
	 * Add a channel group name
	 */
	void add_channel_group_name(const string &channel_group_name);

	/**
	 * Add a signal to the channel. For now only AnalogTimeSignals
	 * are supported.
	 */
	void add_signal(shared_ptr<data::AnalogTimeSignal> signal);

	/**
	 * Add a signal by its quantity, quantity_flags and unit.
	 */
	shared_ptr<data::BaseSignal> add_signal(
		data::Quantity quantity,
		set<data::QuantityFlag> quantity_flags,
		data::Unit unit,
		string custom_name = "");

	/**
	 * Get the actual signal
	 */
	shared_ptr<data::BaseSignal> actual_signal();

	/**
	 * Get all signals for this channel. Normaly a measurement quantity only
	 * has one corresponding signal, but for user channels this can be
	 * different.
	 */
	map<measured_quantity_t, vector<shared_ptr<data::BaseSignal>>> signal_map();

	/**
	 * Get all signals for this channel.
	 * This function is especially for the Python bindings.
	 */
	vector<shared_ptr<data::BaseSignal>> signals();

	/**
	 * Delete all signals from this channel
	 */
	void clear_signals();

	virtual void save_settings(QSettings &settings) const;
	virtual void restore_settings(QSettings &settings);

protected:
	static const size_t size_of_double_ = sizeof(double);

	/** The corresponding sigrok channel object. */
	shared_ptr<sigrok::Channel> sr_channel_;
	/** Name of this channel. */
	string name_;
	/** Index of this channel. */
	unsigned int index_;
	/** Type of this channel. */
	ChannelType type_;
	/** Timestamp when this channel was created/started. */
	double channel_start_timestamp_;

	/** The device this channel belongs to. */
	shared_ptr<devices::BaseDevice> parent_device_;
	/** The channel group names this channel belongs to. */
	set<string> channel_group_names_;

	bool fixed_signal_;
	shared_ptr<data::BaseSignal> actual_signal_;
	map<measured_quantity_t, vector<shared_ptr<data::BaseSignal>>> signal_map_;

public Q_SLOTS:
	void on_aquisition_start_timestamp_changed(double timestamp);

Q_SIGNALS:
	void channel_start_timestamp_changed(double timestamp);
	void enabled_changed(const bool enabled);
	void name_changed(const std::string &name);
	void signal_added(shared_ptr<sv::data::BaseSignal> signal);
	void signal_changed(shared_ptr<sv::data::BaseSignal> signal);

};

} // namespace channels
} // namespace sv

#endif // CHANNELS_BASECHANNEL_HPP
