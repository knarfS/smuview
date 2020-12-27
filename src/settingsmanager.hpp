/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef SETTINGSMANAGER_HPP
#define SETTINGSMANAGER_HPP

#include <memory>
#include <string>

#include <QSettings>
#include <QString>

using std::shared_ptr;
using std::string;

namespace sv {

class Session;

namespace channels {
class BaseChannel;
}
namespace data {
namespace properties {
class BaseProperty;
}
class BaseSignal;
}
namespace devices {
class BaseDevice;
class Configurable;
}

class SettingsManager
{

public:
	SettingsManager();

	/**
	 * Replace some special character (like ':', '/' or '\'), to use the given
	 * string as a settings key.
	 *
	 * @param[in] str The string to format.
	 *
	 * @return The formated key.
	 */
	static string format_key(const string &str);

	/**
	 * Check if setting should be restored. This is controlled by the command
	 * line option -c.
	 *
	 * @return true if a setting should be restored.
	 */
	static bool restore_settings();

	/**
	 * Set if the setting should be restored. This is used by the command
	 * line option -c.
	 *
	 * @param[in] restore_settings The restore_settings flag.
	 */
	static void set_restore_settings(bool restore_settings);

	/**
	 * Check if a setting for this device exists.
	 *
	 * @param[in] device The device.
	 *
	 * @return true if a setting exists.
	 */
	static bool has_device_settings(shared_ptr<sv::devices::BaseDevice> device);

	/**
	 * Save the configurable to the settings.
	 *
	 * @param[in] configurable The configurable to save.
	 * @param[in] settings The settings.
	 * @param[in] origin_device The origin device, this settings belongs to.
	 * @param[in] key_prefix The prefix of the settings keys for the configurable.
	 */
	static void save_configurable(
		const shared_ptr<sv::devices::Configurable> &configurable,
		QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device,
		const QString &key_prefix = "");

	/**
	 * Save the channel to the settings.
	 *
	 * @param[in] channel The channel to save.
	 * @param[in] settings The settings.
	 * @param[in] origin_device The origin device, this settings belongs to.
	 * @param[in] key_prefix The prefix of the settings keys for the channel.
	 */
	static void save_channel(
		const shared_ptr<sv::channels::BaseChannel> &channel,
		QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device,
		const QString &key_prefix = "");

	/**
	 * Save the signal to the settings.
	 *
	 * @param[in] signal The signal to save.
	 * @param[in] settings The settings.
	 * @param[in] origin_device The origin device, this settings belongs to.
	 * @param[in] key_prefix The prefix of the settings keys for the signal.
	 */
	static void save_signal(
		const shared_ptr<sv::data::BaseSignal> &signal,
		QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device,
		const QString &key_prefix = "");

	/**
	 * Save the property to the settings.
	 *
	 * @param[in] property The property to save.
	 * @param[in] settings The settings.
	 * @param[in] origin_device The origin device, this settings belongs to.
	 * @param[in] key_prefix The prefix of the settings keys for the property.
	 */
	static void save_property(
		const shared_ptr<sv::data::properties::BaseProperty> &property,
		QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device,
		const QString &key_prefix = "");

	/**
	 * Restore a device from the settings.
	 *
	 * @param[in] session The SmuView session.
	 * @param[in] settings The settings.
	 * @param[in] origin_device The origin device, this settings belongs to.
	 * @param[in] key_prefix The prefix of the settings keys for the device.
	 *
	 * @return The restored device.
	 */
	static shared_ptr<sv::devices::BaseDevice> restore_device(
		Session &session, QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device,
		const QString &key_prefix = "");

	/**
	 * Restore a configurable from the settings.
	 *
	 * @param[in] session The SmuView session.
	 * @param[in] settings The settings.
	 * @param[in] origin_device The origin device, this settings belongs to.
	 * @param[in] key_prefix The prefix of the settings keys for the configurable.
	 *
	 * @return The restored configurable.
	 */
	static shared_ptr<sv::devices::Configurable> restore_configurable(
		Session &session, QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device,
		const QString &key_prefix = "");

	/**
	 * Restore a property from the settings.
	 *
	 * @param[in] session The SmuView session.
	 * @param[in] settings The settings.
	 * @param[in] origin_device The origin device, this settings belongs to.
	 * @param[in] key_prefix The prefix of the settings keys for the property.
	 *
	 * @return The restored property.
	 */
	static shared_ptr<sv::data::properties::BaseProperty> restore_property(
		Session &session, QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device,
		const QString &key_prefix = "");

	/**
	 * Restore a channel from the settings.
	 *
	 * @param[in] session The SmuView session.
	 * @param[in] settings The settings.
	 * @param[in] origin_device The origin device, this settings belongs to.
	 * @param[in] key_prefix The prefix of the settings keys for the channel.
	 *
	 * @return The restored channel.
	 */
	static shared_ptr<sv::channels::BaseChannel> restore_channel(
		Session &session, QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device,
		const QString &key_prefix = "");

	/**
	 * Restore a signal from the settings.
	 *
	 * @param[in] session The SmuView session.
	 * @param[in] settings The settings.
	 * @param[in] origin_device The origin device, this settings belongs to.
	 * @param[in] key_prefix The prefix of the settings keys for the signal.
	 *
	 * @return The restored signal.
	 */
	static shared_ptr<sv::data::BaseSignal> restore_signal(
		Session &session, QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device,
		const QString &key_prefix = "");

private:
	static bool restore_settings_;

};

} // namespace sv

#endif // SETTINGSMANAGER_HPP
