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

#ifndef WIDGETS_LCDDISPLAY_HPP
#define WIDGETS_LCDDISPLAY_HPP

#include <QFrame>
#include <QLCDNumber>
#include <QLabel>

namespace sv {
namespace widgets {

class LcdDisplay : public QFrame
{
    Q_OBJECT

private:
	uint digits_;
	QString unit_;
	float value_;

	QLCDNumber *lcdValue;
	QLabel *lcdUnit;

	void setup_ui();

public:
	LcdDisplay(const uint digits, const QString unit, QWidget *parent);

	void set_value(const double value);
	void reset_value();
};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_LCDDISPLAY_HPP
