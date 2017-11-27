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

#ifndef WIDGETS_LED_HPP
#define WIDGETS_LED_HPP

#include <memory>

#include <QIcon>
#include <QLabel>
#include <QWidget>

using std::shared_ptr;

namespace sv {

namespace devices {
class HardwareDevice;
}

namespace widgets {

class Led : public QWidget
{
    Q_OBJECT

public:
	Led(bool (devices::HardwareDevice::*get_state_caller)() const,
		bool (devices::HardwareDevice::*is_getable_caller)() const,
		shared_ptr<devices::HardwareDevice> device,
		QString text,
		QWidget *parent = 0);

private:
	bool is_enabled_;
	bool state_;

	bool (devices::HardwareDevice::*get_state_caller_)() const;
	bool (devices::HardwareDevice::*is_getable_caller_)() const;
	shared_ptr<devices::HardwareDevice> device_;

	QString text_;

	QLabel *ledLabel_;
	QLabel *textLabel_;

	QIcon icon_red_;
	QIcon icon_green_;
	QIcon icon_grey_;

	void setup_ui();

public Q_SLOTS:
	void on_state_changed(const bool enabled);

};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_LED_HPP

