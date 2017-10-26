/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2015 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#ifndef DEVICES_DEVICE_HPP
#define DEVICES_DEVICE_HPP

#include <memory>
#include <string>
#include <unordered_set>

using std::map;
using std::mutex;
using std::recursive_mutex;
using std::shared_ptr;
using std::string;
using std::unordered_set;
using std::vector;

namespace sigrok {
class ConfigKey;
class Device;
class Session;
}

namespace sv {

class DeviceManager;

namespace data {
class Analog;
class AnalogSegment;
class SignalBase;
class SignalData;
}

namespace devices {

class Device : public QObject
{
	Q_OBJECT

protected:
	Device() = default;

public:
	virtual ~Device();

	shared_ptr<sigrok::Device> sr_device() const;

	template<typename T>
	T read_config(const sigrok::ConfigKey *key, const T default_value = 0);

	/**
	 * Builds the full name. It only contains all the fields.
	 */
	virtual string full_name() const = 0;

	/**
	 * Builds the display name. It only contains fields as required.
	 * @param device_manager a reference to the device manager is needed
	 * so that other similarly titled devices can be detected.
	 */
	virtual string display_name(
		const DeviceManager &device_manager) const = 0;

	virtual void open() = 0;
	virtual void close() = 0;

	void update_signals();
	void data_feed_in(shared_ptr<sigrok::Device> sr_device,
		shared_ptr<sigrok::Packet> sr_packet);

private:
	unordered_set< shared_ptr<data::SignalBase> > signalbases_;
	unordered_set< shared_ptr<data::SignalData> > all_signal_data_;

	mutable recursive_mutex data_mutex_;
	shared_ptr<data::Analog> analog_data_;
	uint64_t cur_samplerate_;
	map< shared_ptr<sigrok::Channel>, shared_ptr<data::AnalogSegment> >
		cur_analog_segments_;

	vector<shared_ptr<double>> data_ch1_;
	vector<shared_ptr<double>> data_ch2_;
	vector<shared_ptr<double>> time_;

	bool out_of_memory_;
	bool frame_began_;

	shared_ptr<data::SignalBase> signalbase_from_channel(
		shared_ptr<sigrok::Channel> channel) const;
	void feed_in_header();
	void feed_in_meta(shared_ptr<sigrok::Meta> meta);
	void feed_in_trigger();
	void feed_in_frame_begin();
	void feed_in_frame_end();
	void feed_in_analog(shared_ptr<sigrok::Analog> sr_analog);

protected:
	shared_ptr<sigrok::Device> sr_device_;

// TODO: move to hardwaredevice
Q_SIGNALS:
	void data_received(const shared_ptr<sv::data::AnalogSegment>);
	void enabled_changed(const bool);
	void voltage_target_changed(const double);
	void current_limit_changed(const double);
	void over_voltage_protection_active_changed(const bool);
	void over_current_protection_active_changed(const bool);
	void under_voltage_condition_active_changed(const bool);
	void over_temperature_protection_active_changed(const bool);
};

} // namespace devices
} // namespace sv

#endif // DEVICES_DEVICE_HPP
