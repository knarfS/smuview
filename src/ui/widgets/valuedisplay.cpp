/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <limits>

#include <QFrame>
#include <QString>

#include "valuedisplay.hpp"
#include "src/util.hpp"

namespace sv {
namespace ui {
namespace widgets {

ValueDisplay::ValueDisplay(
		int digits, int decimal_places, const bool auto_range,
		const QString unit, const QString unit_suffix, const QString extra_text,
		const bool small, QWidget *parent) :
	QFrame(parent),
	digits_(digits),
	decimal_places_(decimal_places),
	digits_changed_(true),
	auto_range_(auto_range),
	extra_text_(extra_text),
	extra_text_changed_(true),
	unit_(unit),
	unit_si_prefix_(""),
	unit_suffix_(unit_suffix),
	unit_changed_(true),
	small_(small),
	value_(.0)
{
}

double ValueDisplay::value() const
{
	return value_;
}

void ValueDisplay::set_value(const double value)
{
	value_ = value;
	update_display();
}

void ValueDisplay::set_extra_text(const QString extra_text)
{
	extra_text_ = extra_text;
	extra_text_changed_ = true;
	update_display();
}

void ValueDisplay::set_unit(const QString unit)
{
	unit_ = unit;
	unit_changed_ = true;
	update_display();
}

void ValueDisplay::set_unit_suffix(const QString unit_suffix)
{
	unit_suffix_ = unit_suffix;
	unit_changed_ = true;
	update_display();
}

void ValueDisplay::set_digits(const int digits, const int decimal_places)
{
	digits_ = digits;
	decimal_places_ = decimal_places;
	digits_changed_ = true;
	update_display();
}

void ValueDisplay::reset_value()
{
	QString init_value("");
	for (int i=0; i<digits_; i++)
		init_value.append("-");
	show_value(init_value);
}

void ValueDisplay::update_display()
{
	QString value_str("");
	QString si_prefix("");

	if (value_ >= std::numeric_limits<double>::max() ||
			value_ == std::numeric_limits<double>::infinity()) {
		// TODO: Replace with "ol" or "overl", depending on the avail. digits.
		value_str = QString("OL");
	}
	else if (value_ <= std::numeric_limits<double>::lowest()) {
		// TODO: Replace with "ul" or "underf", depending on the avail. digits.
		value_str = QString("UL");
	}
	else if (!auto_range_) {
		// Use actual locale (%L) for formating
		value_str = QString("%L1").
			arg(value_, digits_, 'f', decimal_places_, QChar(' '));
	}
	else {
		util::format_value_si(
			value_, digits_, decimal_places_, value_str, si_prefix);
	}
	show_value(value_str);

	if (digits_changed_) {
		digits_changed_ = false;
		this->update_value_widget_dimensions();
	}

	if (extra_text_changed_) {
		extra_text_changed_ = false;
		show_extra_text(extra_text_);
		this->update_extra_widget_dimensions();
	}

	if (si_prefix != unit_si_prefix_ || unit_changed_) {
		unit_si_prefix_ = si_prefix;
		QString unit_str = QString("%1%2").arg(unit_si_prefix_, unit_);
		if (!unit_suffix_.isEmpty()) {
			unit_str.append(" ").append(unit_suffix_);
		}
		show_unit(unit_str);
	}
	if (unit_changed_) {
		unit_changed_ = false;
		this->update_unit_widget_dimensions();
	}
}

} // namespace widgets
} // namespace ui
} // namespace sv
