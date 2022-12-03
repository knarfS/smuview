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

#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QColorDialog>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QPushButton>
#include <QWidget>

#include "colorbutton.hpp"

namespace sv {
namespace ui {
namespace widgets {

const int ColorButton::SwatchMargin = 7;

ColorButton::ColorButton(QWidget *parent) :
	QPushButton(parent)
{
	connect(this, SIGNAL(clicked()), this, SLOT(change_color()));
}

void ColorButton::set_color(const QColor &color)
{
	color_ = color;
}

const QColor &ColorButton::color()
{
	return color_;
}

void ColorButton::change_color()
{
	QColor new_color = QColorDialog::getColor(color_, parentWidget());
	if (new_color != color_)
		set_color(new_color);
}

void ColorButton::paintEvent(QPaintEvent *event)
{
	QPushButton::paintEvent(event);

	const QRect rect_adjusted = rect().adjusted(
		SwatchMargin, SwatchMargin, -SwatchMargin, -SwatchMargin);

	QPainter painter(this);
	painter.setPen(QApplication::palette().color(QPalette::Dark));
	painter.setBrush(QBrush(color_));
	painter.drawRect(rect_adjusted);
}

} // namespace widgets
} // namespace ui
} // namespace sv
