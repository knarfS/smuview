/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef WIDGETS_CONTROLBUTTON_HPP
#define WIDGETS_CONTROLBUTTON_HPP

#include <memory>

#include <QPushButton>

using std::shared_ptr;

namespace sv {

namespace devices {
class HardwareDevice;
}

namespace widgets {

class ControlButton : public QPushButton
{
    Q_OBJECT

public:
	ControlButton(
		bool (devices::HardwareDevice::*get_state_caller)() const,
		void (devices::HardwareDevice::*set_state_caller)(double),
		bool (devices::HardwareDevice::*is_getable_caller)() const,
		bool (devices::HardwareDevice::*is_setable_caller)() const,
		shared_ptr<devices::HardwareDevice> device, QWidget *parent = 0);

private:
	bool state_;
	bool is_readable_;
	bool is_setable_;

	bool (devices::HardwareDevice::*get_state_caller_)() const;
	void (devices::HardwareDevice::*set_state_caller)(double);
	bool (devices::HardwareDevice::*is_getable_caller_)() const;
	bool (devices::HardwareDevice::*is_setable_caller)() const;
	shared_ptr<devices::HardwareDevice> device_;

	QIcon icon_red_;
	QIcon icon_green_;
	QIcon icon_grey_;

	void setup_ui();

public Q_SLOTS:
	void on_clicked();
	void on_state_changed(const bool enabled);

Q_SIGNALS:
	void state_changed(const bool enabled);
};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_CONTROLBUTTON_HPP

