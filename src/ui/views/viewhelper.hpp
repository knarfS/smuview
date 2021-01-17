/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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
#include <vector>

#include <QSettings>

using std::shared_ptr;
using std::string;
using std::vector;

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
class Configurable;
class BaseDevice;
}

namespace ui {
namespace views {

class BaseView;

namespace viewhelper {

/**
 * Returns the fitting control views for the given configurable, by checking for
 * get-/setable config keys and the device type.
 *
 * @param[in] session The reference to the actual SmuView session.
 * @param[in] configurable The Configurable.
 *
 * @return The control views for the configurable.
 */
vector<BaseView *> get_views_for_configurable(Session &session,
	shared_ptr<sv::devices::Configurable> configurable);

/**
 * Return the view defined in the actual settings group.
 *
 * @param[in] session The reference to the actual SmuView session.
 * @param[in] settings The settings.
 * @param[in] origin_device The origin device for this settings.
 *
 * @return The view defined by settings.
 */
BaseView *get_view_from_settings(Session &session, QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device);

} // namespace viewhelper
} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_VIEWHELPER_HPP
