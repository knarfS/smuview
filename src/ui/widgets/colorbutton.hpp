/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_COLORBUTTON_HPP
#define UI_WIDGETS_COLORBUTTON_HPP

#include <QColor>
#include <QPaintEvent>
#include <QPushButton>
#include <QWidget>

namespace sv {
namespace ui {
namespace widgets {

class ColorButton : public QPushButton
{
	Q_OBJECT

public:
	explicit ColorButton(QWidget *parent = nullptr);

	void set_color(const QColor &color);
	const QColor &color();

private:
	static const int SwatchMargin;
	QColor color_;

private:
	void paintEvent(QPaintEvent *event) override;

public Q_SLOTS:
	void change_color();

};

} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_COLORBUTTON_HPP
