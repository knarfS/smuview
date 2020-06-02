/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_VIEWHELPER_HPP
#define UI_VIEWS_VIEWHELPER_HPP

#include <memory>
#include <string>

#include <QSettings>

using std::shared_ptr;
using std::string;

namespace sv {

class Session;

namespace channels {
class BaseChannel;
}

namespace data {
class AnalogTimeSignal;
class BaseSignal;
}

namespace devices {
class Configurable;
class BaseDevice;
}

namespace ui {
namespace views {

class BaseView;

namespace viewhelper {

/**
 * Returns the fitting control view for the given configurable, by checking for
 * get-/setable config keys and the device type.
 *
 * @param[in] session The reference to the actual SmuView session.
 * @param[in] configurable The Configurable.
 *
 * @return The control view for the configurable.
 */
BaseView *get_view_for_configurable(Session &session,
	shared_ptr<sv::devices::Configurable> configurable);

/**
 * Check if a setting for this device exists.
 *
 * @param[in] device The device.
 *
 * @return true if a setting exists.
 */
bool has_device_settings(shared_ptr<sv::devices::BaseDevice> device);

/**
 * Return the view defined in the actual settings group.
 *
 * @param[in] session The reference to the actual SmuView session.
 * @param[in] settings The settings.
 *
 * @return The view defined by settings.
 */
BaseView *get_view_from_settings(Session &session, QSettings &settings);

/**
 * Save the configurable to the settings.
 *
 * @param[in] configurable The configurable to save.
 * @param[in] settings The settings.
 * @param[in] key_prefix The prefix of the settings keys for the configurable.
 */
void save_configurable(
	const shared_ptr<sv::devices::Configurable> &configurable,
	QSettings &settings, const QString &key_prefix = "");

/**
 * Save the channel to the settings.
 *
 * @param[in] channel The channel to save.
 * @param[in] settings The settings.
 * @param[in] key_prefix The prefix of the settings keys for the channel.
 */
void save_channel(const shared_ptr<sv::channels::BaseChannel> &channel,
	QSettings &settings, const QString &key_prefix = "");

/**
 * Save the signal to the settings.
 *
 * @param[in] signal The signal to save.
 * @param[in] settings The settings.
 * @param[in] key_prefix The prefix of the settings keys for the signal.
 */
void save_signal(const shared_ptr<sv::data::BaseSignal> &signal,
	QSettings &settings, const QString &key_prefix = "");

/**
 * Restore a channel from the settings.
 *
 * @param[in] session The SmuView session.
 * @param[in] settings The settings.
 * @param[in] key_prefix The prefix of the settings keys for the channel.
 *
 * @return The restored channel.
 */
shared_ptr<sv::channels::BaseChannel> restore_channel(Session &session,
	QSettings &settings, const QString &key_prefix = "");

/**
 * Restore a signal from the settings.
 *
 * @param[in] session The SmuView session.
 * @param[in] settings The settings.
 * @param[in] key_prefix The prefix of the settings keys for the signal.
 *
 * @return The restored signal.
 */
shared_ptr<sv::data::AnalogTimeSignal> restore_signal(Session &session,
	QSettings &settings, const QString &key_prefix = "");

} // namespace viewhelper
} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_VIEWHELPER_HPP
