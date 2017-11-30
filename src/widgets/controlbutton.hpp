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

#include <QPushButton>

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
		const bool is_state_getable, const bool is_state_setable,
		QWidget *parent = nullptr);

private:
	bool state_;
	bool is_state_enabled_;
	const bool is_state_getable_;
	const bool is_state_setable_;
	const QIcon on_icon_;
	const QIcon off_icon_;
	const QIcon dis_icon_;

	void setup_ui();
	void connect_signals();

public Q_SLOTS:
	void change_state(const bool state);

private Q_SLOTS:
	void on_state_changed();

Q_SIGNALS:
	void state_changed(const bool);

};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_CONTROLBUTTON_HPP

