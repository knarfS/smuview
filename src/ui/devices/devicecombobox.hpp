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

#ifndef UI_DEVICES_DEVICECOMBOBOX_HPP
#define UI_DEVICES_DEVICECOMBOBOX_HPP

#include <memory>

#include <QComboBox>
#include <QWidget>

using std::shared_ptr;

namespace sv {

class Session;

namespace devices {
class BaseDevice;
}

namespace ui {
namespace devices {

class DeviceComboBox : public QComboBox
{
	Q_OBJECT

public:
	explicit DeviceComboBox(const Session &session, QWidget *parent = nullptr);

	void select_device(shared_ptr<sv::devices::BaseDevice> device);
	shared_ptr<sv::devices::BaseDevice> selected_device() const;

private:
	void setup_ui();

	const Session &session_;

Q_SIGNALS:
	void device_changed();

};

} // namespace devices
} // namespace ui
} // namespace sv

#endif // UI_DEVICES_DEVICECOMBOBOX_HPP
