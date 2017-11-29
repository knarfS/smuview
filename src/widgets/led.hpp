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

#include <QIcon>
#include <QLabel>
#include <QWidget>

namespace sv {

namespace devices {
class HardwareDevice;
}

namespace widgets {

class Led : public QWidget
{
    Q_OBJECT

public:
	Led(const bool is_state_getable,
		QString text, QIcon on_icon, QIcon off_icon, QIcon dis_icon,
		QWidget *parent = 0);

private:
	const bool is_state_getable_;
	const QString text_;
	const QIcon on_icon_;
	const QIcon off_icon_;
	const QIcon dis_icon_;

	QLabel *led_label_;
	QLabel *text_label_;

	void setup_ui();

public Q_SLOTS:
	void change_state(const bool state);

};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_LED_HPP

