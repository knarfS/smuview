/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_VALUEDISPLAY_HPP
#define UI_WIDGETS_VALUEDISPLAY_HPP

#include <QFrame>
#include <QString>

namespace sv {
namespace ui {
namespace widgets {

class ValueDisplay : public QFrame
{
	Q_OBJECT

public:
	ValueDisplay(
		int digits, int decimal_places, const bool auto_range,
		const QString unit, const QString unit_suffix, const QString extra_text,
		const bool small, QWidget *parent = nullptr);

	double value() const;

protected:
	int digits_;
	int decimal_places_;
	bool digits_changed_;
	const bool auto_range_;
	QString extra_text_;
	bool extra_text_changed_;
	QString unit_;
	QString unit_si_prefix_;
	QString unit_suffix_;
	bool unit_changed_;
	const bool small_;
	double value_;

	virtual void setup_ui() = 0;
	virtual void update_value_widget_dimensions() = 0;
	virtual void update_extra_widget_dimensions() = 0;
	virtual void update_unit_widget_dimensions() = 0;
	virtual void show_value(const QString &value) = 0;
	virtual void show_extra_text(const QString &extra_text) = 0;
	virtual void show_unit(const QString &unit) = 0;

public Q_SLOTS:
	void set_value(const double);
	void set_extra_text(const QString);
	void set_unit(const QString);
	void set_unit_suffix(const QString);
	void set_digits(const int, const int);
	void reset_value();
	void update_display();

};

} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_VALUEDISPLAY_HPP
