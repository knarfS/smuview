/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DEVICES_VIRTUALDEVICE_HPP
#define DEVICES_VIRTUALDEVICE_HPP

#include <memory>
#include <string>

#include <QObject>
#include <QString>

#include "src/devices/basedevice.hpp"

using std::shared_ptr;
using std::string;

namespace sigrok {
class Context;
}

namespace sv {

namespace devices {

class VirtualDevice : public BaseDevice
{
	Q_OBJECT

public:
	VirtualDevice(const shared_ptr<sigrok::Context> &sr_context,
		QString vendor, QString model, QString version);

	/**
	 * Inits all configurables for this hardware device. Not used in here!
	 */
	void init()  override;

	/**
	 * Builds the name
	 */
	QString name() const  override;

	/**
	 * Builds the full name. It only contains all the fields.
	 */
	QString full_name() const  override;

	/**
	 * Builds the short name.
	 */
	QString short_name() const  override;

	/**
	 * Get the unique Id of the device
	 */
	string id() const  override;

	/**
	 * Builds the display name. It only contains fields as required.
	 * @param device_manager a reference to the device manager is needed
	 * so that other similarly titled devices can be detected.
	 */
	QString display_name(const DeviceManager &device_manager) const  override;

	void open(function<void (const QString)> error_handler)  override;
	void close()  override;
	void add_channel(shared_ptr<channels::BaseChannel> channel,
		QString channel_group_name)  override;

protected:
	/**
	 * Inits all channles of this hardware device
	 */
	void init_channels()  override;
	void feed_in_header()  override;
	void feed_in_trigger()  override;
	void feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)  override;
	void feed_in_frame_begin()  override;
	void feed_in_frame_end()  override;
	void feed_in_logic(shared_ptr<sigrok::Logic> sr_logic)  override;
	void feed_in_analog(shared_ptr<sigrok::Analog> sr_analog)  override;

private:
	double frame_start_timestamp_;
	QString vendor_;
	QString model_;
	QString version_;
	unsigned int channel_index_;

Q_SIGNALS:
	void channel_changed(QString);

};

} // namespace devices
} // namespace sv

#endif // DEVICES_VIRTUALDEVICE_HPP
