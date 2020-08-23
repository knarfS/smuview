/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <QDebug>
#include <QString>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "basechannel.hpp"
#include "src/util.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::make_pair;
using std::make_shared;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;
using sv::data::measured_quantity_t;

Q_DECLARE_METATYPE(std::shared_ptr<sv::data::BaseSignal>)

namespace sv {
namespace channels {

BaseChannel::BaseChannel(
		shared_ptr<sigrok::Channel> sr_channel,
		shared_ptr<devices::BaseDevice> parent_device,
		const set<string> &channel_group_names,
		double channel_start_timestamp) :
	sr_channel_(sr_channel),
	channel_start_timestamp_(channel_start_timestamp),
	parent_device_(parent_device),
	channel_group_names_(channel_group_names),
	fixed_signal_(false),
	actual_signal_(nullptr)
{
	name_ = (sr_channel_) ? sr_channel_->name() : "";

	qWarning() << "Init channel " << QString::fromStdString(name_)
		<< ", channel_start_timestamp = "
		<< util::format_time_date(channel_start_timestamp);
}

BaseChannel::~BaseChannel()
{
	qWarning() << "BaseChannel::~BaseChannel(): " <<
		QString::fromStdString(name());
}

shared_ptr<sigrok::Channel> BaseChannel::sr_channel() const
{
	return sr_channel_;
}

string BaseChannel::name() const
{
	return (sr_channel_) ? sr_channel_->name() : name_;
}

void BaseChannel::set_name(const string &name)
{
	if (sr_channel_)
		sr_channel_->set_name(name);

	name_ = name;
	Q_EMIT name_changed(name);
}

QString BaseChannel::display_name() const
{
	return QString::fromStdString(name_);
}

unsigned int BaseChannel::index() const
{
	return (sr_channel_) ? sr_channel_->index() : index_;
}

ChannelType BaseChannel::type() const
{
	return type_;
}

bool BaseChannel::enabled() const
{
	return (sr_channel_) ? sr_channel_->enabled() : true;
}

void BaseChannel::set_enabled(bool enabled)
{
	if (sr_channel_) {
		sr_channel_->set_enabled(enabled);
		Q_EMIT enabled_changed(enabled);
	}
}

bool BaseChannel::fixed_signal()
{
	return fixed_signal_;
}

void BaseChannel::set_fixed_signal(bool fixed_signal)
{
	fixed_signal_ = fixed_signal;
}

shared_ptr<devices::BaseDevice> BaseChannel::parent_device()
{
	return parent_device_;
}

set<string> BaseChannel::channel_group_names() const
{
	return channel_group_names_;
}

void BaseChannel::add_channel_group_name(const string &channel_group_name)
{
	channel_group_names_.insert(channel_group_name);
}

void BaseChannel::add_signal(shared_ptr<data::AnalogTimeSignal> signal)
{
	if (!signal_map_.empty() && fixed_signal_) {
		qWarning() << "Warning: Adding new signal " << signal->display_name() <<
			"to fixed channel " << display_name();
		return;
	}

	connect(this, SIGNAL(channel_start_timestamp_changed(double)),
			signal.get(), SLOT(on_channel_start_timestamp_changed(double)));

	measured_quantity_t mq = make_pair(
		signal->quantity(), signal->quantity_flags());
	if (signal_map_.count(mq) > 0) {
		signal_map_[mq].push_back(signal);
	}
	else {
		signal_map_.insert(
			make_pair(mq, vector<shared_ptr<data::BaseSignal>> { signal }));
	}

	actual_signal_ = signal;
	Q_EMIT signal_added(signal);
}

shared_ptr<data::BaseSignal> BaseChannel::add_signal(
	data::Quantity quantity,
	set<data::QuantityFlag> quantity_flags,
	data::Unit unit,
	string custom_name)
{
	/*
	 * TODO: Remove shared_from_this() / (channel pointer in signal), so that
	 *       "add_signal()" can be called from MathChannel ctor.
	 */
	auto signal = make_shared<data::AnalogTimeSignal>(
		quantity, quantity_flags, unit,
		shared_from_this(), channel_start_timestamp_, custom_name);

	this->add_signal(signal);

	return signal;
}

shared_ptr<data::BaseSignal> BaseChannel::actual_signal()
{
	return actual_signal_;
}

map<measured_quantity_t, vector<shared_ptr<data::BaseSignal>>>
	BaseChannel::signal_map()
{
	return signal_map_;
}

vector<shared_ptr<data::BaseSignal>> BaseChannel::signals()
{
	vector<shared_ptr<data::BaseSignal>> signals;
	for (const auto &signal_pair : signal_map_) {
		signals.insert(signals.end(),
			signal_pair.second.begin(), signal_pair.second.end());
	}
	return signals;
}

void BaseChannel::clear_signals()
{
	/* TODO
	for (const auto &signal_pair : signal_map_) {
		signal_pair.second->
	}
	*/
}

void BaseChannel::save_settings(QSettings &settings) const
{
	settings.setValue("name", QString::fromStdString(name()));
	settings.setValue("enabled", enabled());
}

void BaseChannel::restore_settings(QSettings &settings)
{
	set_name(settings.value("name").toString().toStdString());
	set_enabled(settings.value("enabled").toBool());
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
