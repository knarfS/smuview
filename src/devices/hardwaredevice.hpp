/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2015 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DEVICES_HARDWAREDEVICE_HPP
#define DEVICES_HARDWAREDEVICE_HPP

#include <map>
#include <mutex>
#include <string>
#include <unordered_set>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QString>

#include "src/devices/basedevice.hpp"

using std::bad_alloc;
using std::dynamic_pointer_cast;
using std::lock_guard;
using std::make_shared;
using std::set;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::vector;
using std::unique_ptr;

using std::map;
using std::mutex;
using std::recursive_mutex;
using std::set;
using std::shared_ptr;
using std::string;
using std::unordered_set;

namespace sigrok {
class Channel;
class Configurable;
class Context;
class Quantity;
class HardwareDevice;
}

namespace sv {

namespace channels {
class BaseChannel;
}
namespace data {
namespace properties {
class UInt64Property;
}
}

namespace devices {

class Configurable;

class HardwareDevice : public BaseDevice
{
	Q_OBJECT

protected:
	HardwareDevice(const shared_ptr<sigrok::Context> sr_context,
		shared_ptr<sigrok::HardwareDevice> sr_device);

public:
	/**
	 * Returns the sigrok hardware device
	 */
	shared_ptr<sigrok::HardwareDevice> sr_hardware_device() const;

	/**
	 * Builds the display name. It only contains fields as required.
	 * @param device_manager a reference to the device manager is needed
	 * so that other similarly titled devices can be detected.
	 */
	QString display_name(const DeviceManager &device_manager) const override;

	void open() override;

protected:
	/**
	 * Init all configurables for this hardware device.
	 */
	void init_configurables() override;
	/**
	 * Init all sigrok channels for this hardware device.
	 */
	void init_channels() override;

	void feed_in_header() override;
	void feed_in_trigger() override;
	void feed_in_meta(shared_ptr<sigrok::Meta> sr_meta) override;
	void feed_in_frame_begin() override;
	void feed_in_frame_end() override;
	void feed_in_logic(shared_ptr<sigrok::Logic> sr_logic) override;
	void feed_in_analog(shared_ptr<sigrok::Analog> sr_analog) override;

private:
	/**
	 * Indicates, if a new frame has been opened by SR_DF_FRAME_BEGIN, but not
	 * closed yet by a SR_DF_FRAME_END packet.
	 */
	bool frame_began_;
	/** Timestamp when the frame was opened. */
	double frame_start_timestamp_;
	uint64_t cur_samplerate_;
	uint64_t cur_sample_interval_;

};

} // namespace devices
} // namespace sv

#endif // DEVICES_HARDWAREDEVICE_HPP
