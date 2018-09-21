/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_LCDDISPLAY_HPP
#define UI_WIDGETS_LCDDISPLAY_HPP

#include <QFrame>
#include <QLCDNumber>
#include <QLabel>

namespace sv {
namespace ui {
namespace widgets {

class LcdDisplay : public QFrame
{
    Q_OBJECT

public:
	LcdDisplay(
		int digits, int decimal_places, const bool auto_range,
		const QString unit, const QString unit_suffix, const QString extra_text,
		const bool small, QWidget *parent = nullptr);

	double value() const;

private:
	int digits_;
	int decimal_places_;
	const bool auto_range_;
	QString unit_;
	QString unit_si_prefix_;
	QString unit_suffix_;
	bool update_unit_;
	QString extra_text_;
	const bool small_;
	double value_;
	uint height_;
	uint width_scale_factor_;

	QLCDNumber *lcd_value_;
	QLabel *lcd_extra_;
	QLabel *lcd_unit_;

	void setup_ui();
	void update_size();

public Q_SLOTS:
	void set_value(const double);
	void set_unit(const QString);
	void set_unit_suffix(const QString);
	void set_extra_text(const QString);
	void set_digits(const int, const int);
	void reset_value();
	void update_display();

};

} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_LCDDISPLAY_HPP
