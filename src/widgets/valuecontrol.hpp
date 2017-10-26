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

#ifndef WIDGETS_VALUECONTROL_HPP
#define WIDGETS_VALUECONTROL_HPP

#include <QWidget>
#include <QDoubleSpinBox>
#include <qwt_knob.h>

#include "lcddisplay.hpp"

namespace sv {
namespace widgets {

class ValueControl : public QWidget
{
    Q_OBJECT

public:
	ValueControl(const uint digits, const QString unit,
		const double min, const double max, const double steps, QWidget *parent);

private:
	double value_;
	uint digits_;
	QString unit_;
	double min_;
	double max_;
	double steps_;

    QDoubleSpinBox *doubleSpinBox;
    QwtKnob *knob;
	LcdDisplay *lcdDisplay;

	void setup_ui();

public Q_SLOTS:
	void change_value(const double value);
	void on_value_changed(const double value);

Q_SIGNALS:
	void value_changed(double value);
};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_VALUECONTROL_HPP
