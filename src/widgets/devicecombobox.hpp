/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef WIDGETS_DEVICECOMBOBOX_HPP
#define WIDGETS_DEVICECOMBOBOX_HPP

#include <memory>

#include <QComboBox>
#include <QWidget>

using std::shared_ptr;

namespace sv {

class Session;

namespace devices {
class Device;
}

namespace widgets {

class DeviceComboBox : public QComboBox
{
    Q_OBJECT

public:
	DeviceComboBox(const Session &session, QWidget *parent = nullptr);

	void select_device(shared_ptr<devices::Device> device);
	const shared_ptr<devices::Device> selected_device();

private:
	void setup_ui();

	const Session &session_;

};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_DEVICECOMBOBOX_HPP

