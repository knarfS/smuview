/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2022 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_MONOFONTDISPLAY_HPP
#define UI_WIDGETS_MONOFONTDISPLAY_HPP

#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QString>

namespace sv {
namespace ui {
namespace widgets {

enum class MonoFontDisplayType {
	AutoRange,
	AutoRangeWithSRDigits,
	FixedRange
};

class MonoFontDisplay : public QFrame
{
	Q_OBJECT

public:
	MonoFontDisplay(const MonoFontDisplayType display_type,
		const QString &unit, const QString &unit_suffix,
		const QString &extra_text, const bool small, QWidget *parent = nullptr);

	double value() const;

private:
	const MonoFontDisplayType display_type_;
	int total_digits_;
	bool total_digits_changed_;
	int sr_digits_;
	int decimal_places_;
	QString extra_text_;
	bool extra_text_changed_;
	QString unit_;
	QString unit_si_prefix_;
	QString unit_suffix_;
	bool unit_changed_;
	const bool small_;
	double value_;
	int ascent_diff_;
	QGridLayout *layout_;
	QLabel *value_label_;
	QFont extra_font_;
	QLabel *extra_label_;
	QSpacerItem *extra_spacer_;
	QLabel *unit_label_;

	virtual void setup_ui();
	void update_value_widget_dimensions();
	void update_extra_widget_dimensions();
	void update_unit_widget_dimensions();
	void show_value(const QString &value);
	void show_extra_text(const QString &extra_text);
	void show_unit(const QString &unit);

public Q_SLOTS:
	void set_value(const double value);
	void set_extra_text(const QString &extra_text);
	void set_unit(const QString &unit);
	void set_unit_suffix(const QString &unit_suffix);
	void set_sr_digits(const int total_digits, const int sr_digits);
	void set_decimal_places(const int total_digits, const int decimal_places);
	void reset_value();
	void update_display();

};

} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_MONOFONTDISPLAY_HPP
