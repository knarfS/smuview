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

#ifndef WIDGETS_COLORBUTTON_HPP
#define WIDGETS_COLORBUTTON_HPP

#include <QColor>
#include <QPushButton>

namespace sv {
namespace widgets {

class ColorButton : public QPushButton
{
    Q_OBJECT

public:
	ColorButton(QWidget *parent = nullptr);

    void set_color(const QColor &color);
    const QColor &color();

private:
	QColor color_;

public Q_SLOTS:
    void update_color();
    void change_color();

};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_COLORBUTTON_HPP


