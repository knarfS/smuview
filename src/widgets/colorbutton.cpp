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

#include <QColorDialog>

#include "colorbutton.hpp"

namespace sv {
namespace widgets {

ColorButton::ColorButton(QWidget *parent) :
	QPushButton(parent)
{
    connect(this, SIGNAL(clicked()), this, SLOT(change_color()));
}

void ColorButton::set_color(const QColor &color)
{
    color_ = color;
    update_color();
}

const QColor &ColorButton::color()
{
    return color_;
}

void ColorButton::update_color()
{
    setStyleSheet("background-color: " + color_.name());
}

void ColorButton::change_color()
{
    QColor new_color = QColorDialog::getColor(color_, parentWidget());
    if (new_color != color_)
        set_color(new_color);
}

} // namespace widgets
} // namespace sv


