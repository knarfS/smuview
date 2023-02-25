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

#include <limits>

#include <QDebug>
#include <QFont>
#include <QFrame>
#include <QFontMetrics>
#include <QGridLayout>
#include <QSizePolicy>
#include <QString>

#include <QStandardPaths>

#include "monofontdisplay.hpp"
#include "src/util.hpp"

namespace sv {
namespace ui {
namespace widgets {

MonoFontDisplay::MonoFontDisplay(const MonoFontDisplayType display_type,
		const QString &unit, const QString &unit_suffix,
		const QString &extra_text, const bool small, QWidget *parent) :
	QFrame(parent),
	display_type_(display_type),
	total_digits_(7),
	total_digits_changed_(true),
	sr_digits_(2),
	decimal_places_(2),
	extra_text_(extra_text),
	extra_text_changed_(true),
	unit_(unit),
	unit_si_prefix_(""),
	unit_suffix_(unit_suffix),
	unit_changed_(true),
	small_(small),
	value_(.0)
{
	MonoFontDisplay::setup_ui();
	reset_value();
}

void MonoFontDisplay::setup_ui()
{
	// Use embedded mono space font
	QFont monospace_font = QFont("DejaVu Sans Mono");
	// Get standard font sizes
	int monospace_font_size = monospace_font.pointSize();
	int std_font_size = QFont().pointSize();

	QFont value_font(monospace_font);
	QFont unit_font;
	int unit_spacer_size;
	if (!small_) {
		value_font.setPointSize(monospace_font_size + 12); // = 22
		value_font.setBold(true);
		value_font.setWeight(QFont::Bold);
		unit_font.setPointSize(std_font_size + 8); // = 18
		extra_font_.setPointSize(std_font_size); // = 10
		unit_spacer_size = 5;
	}
	else {
		value_font.setPointSize(monospace_font_size + 4); // = 14
		unit_font.setPointSize(std_font_size); // = 10
		extra_font_.setPointSize(std_font_size - 3); // = 7
		unit_spacer_size = 3;
	}

	// Qt::AlignBaseline is not working, so we have to calculate the
	// difference of the ascents for positioning the unit label.
	QFontMetrics value_font_metrics(value_font);
	QFontMetrics unit_font_metrics(unit_font);
	ascent_diff_ = value_font_metrics.ascent() - unit_font_metrics.ascent();


	//this->setFrameShape(QFrame::Box);
	QSizePolicy layout_size_policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	layout_size_policy.setHorizontalStretch(0);
	layout_size_policy.setVerticalStretch(0);
	this->setSizePolicy(layout_size_policy);

	layout_ = new QGridLayout();
	// Set the margin and spacing to 0, so we can position the value and
	// the unit by their baselines exactly.
	layout_->setMargin(0);
	layout_->setSpacing(0);

	// Value
	value_label_ = new QLabel();
	value_label_->setFont(value_font);
	value_label_->setAlignment(Qt::AlignRight);
	value_label_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//value_label_->setFrameShape(QFrame::Box);
	layout_->addWidget(value_label_, 0, 0, 2, 1, Qt::AlignRight | Qt::AlignVCenter);

	// Spacer between the value and the unit labels.
	QSpacerItem *unit_spacer = new QSpacerItem(unit_spacer_size, 1,
		QSizePolicy::Fixed, QSizePolicy::Fixed);
	layout_->addItem(unit_spacer, 0, 1, 2, 1, Qt::AlignCenter);

	// Extra spacer (used when extra text is empty to "fake" Qt::AlignBaseline)
	extra_label_ = nullptr;
	extra_spacer_ = new QSpacerItem(1, ascent_diff_,
		QSizePolicy::Fixed, QSizePolicy::Fixed);
	layout_->addItem(extra_spacer_, 0, 2, 1, 1, Qt::AlignCenter);

	// Unit
	unit_label_ = new QLabel();
	unit_label_->setFont(unit_font);
	// Qt::AlignTop is not working!
	unit_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	unit_label_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//unit_label_->setFrameShape(QFrame::Box);
	layout_->addWidget(unit_label_, 1, 2, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
	layout_->setRowStretch(1, 1);

	this->setLayout(layout_);
}

void MonoFontDisplay::update_value_widget_dimensions()
{
	// Set the widget to a fixed width, so it doesn't jump around when the
	// length of the string is changing (e.g. minus sign).
	// TODO: Displays are too small (not wide enough) for neg. values, esp. for
	//       the power panel for W/Ah/Wh for the 6632B. B/c of the decimal
	//       point?
	QString str("");
	size_t str_len = total_digits_ + 2; // digits + decimal point + sign
	for (size_t i=0; i<str_len; ++i) {
		str += "-";
	}
	QFontMetrics metrics = value_label_->fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	value_label_->setFixedWidth(metrics.horizontalAdvance(str));
#else
	value_label_->setFixedWidth(metrics.width(str));
#endif
}

void MonoFontDisplay::update_extra_widget_dimensions()
{
	// Nothing to do here
}

void MonoFontDisplay::update_unit_widget_dimensions()
{
	// Set the widget to a fixed width, so it doesn't jump around when the
	// SI prefix is changing.
	QString str;
	if (display_type_ == MonoFontDisplayType::AutoRangeWithSRDigits
			|| display_type_ == MonoFontDisplayType::AutoRange) {
		// 'm' is the widest character for non monospace fonts
		str.append("m");
	}
	str.append(unit_);
	if (!unit_suffix_.isEmpty()) {
		str.append(" ").append(unit_suffix_);
	}
	QFontMetrics metrics = unit_label_->fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	unit_label_->setFixedWidth(metrics.horizontalAdvance(str));
#else
	unit_label_->setFixedWidth(metrics.width(str));
#endif
}

void MonoFontDisplay::show_value(const QString &value)
{
	value_label_->setText(value);
}

void MonoFontDisplay::show_extra_text(const QString &extra_text)
{
	if (extra_text_.isEmpty() && !extra_spacer_) {
		// Remove label
		layout_->removeWidget(extra_label_);
		delete extra_label_;
		extra_label_ = nullptr;
		// Insert spacer
		extra_spacer_ = new QSpacerItem(1, ascent_diff_,
			QSizePolicy::Fixed, QSizePolicy::Fixed);
		layout_->addItem(extra_spacer_, 0, 2, 1, 1, Qt::AlignCenter);
	}
	else if (!extra_text_.isEmpty()) {
		if (!extra_label_) {
			// Remove spacer
			layout_->removeItem(extra_spacer_);
			delete extra_spacer_;
			extra_spacer_ = nullptr;
			// Insert label
			extra_label_ = new QLabel();
			extra_label_->setFont(extra_font_);
			extra_label_->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
			extra_label_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			//extra_label_->setFrameShape(QFrame::Box);
			layout_->addWidget(extra_label_, 0, 2, 1, 1, Qt::AlignCenter);
		}
		extra_label_->setText(extra_text);
	}
}

void MonoFontDisplay::show_unit(const QString &unit)
{
	unit_label_->setText(unit);
}

double MonoFontDisplay::value() const
{
	return value_;
}

void MonoFontDisplay::set_value(const double value)
{
	value_ = value;
	update_display();
}

void MonoFontDisplay::set_extra_text(const QString &extra_text)
{
	extra_text_ = extra_text;
	extra_text_changed_ = true;
	update_display();
}

void MonoFontDisplay::set_unit(const QString &unit)
{
	unit_ = unit;
	unit_changed_ = true;
	update_display();
}

void MonoFontDisplay::set_unit_suffix(const QString &unit_suffix)
{
	unit_suffix_ = unit_suffix;
	unit_changed_ = true;
	update_display();
}

void MonoFontDisplay::set_sr_digits(const int total_digits, const int sr_digits)
{
	if (total_digits != total_digits_) {
		total_digits_ = total_digits;
		total_digits_changed_ = true;
	}
	sr_digits_ = sr_digits;
	update_display();
}

void MonoFontDisplay::set_decimal_places(const int total_digits, const int decimal_places)
{
	if (total_digits != total_digits_) {
		total_digits_ = total_digits;
		total_digits_changed_ = true;
	}
	decimal_places_ = decimal_places;
	update_display();
}

void MonoFontDisplay::reset_value()
{
	QString init_value("");
	for (int i=0; i<total_digits_; i++)
		init_value.append("-");
	show_value(init_value);
}

void MonoFontDisplay::update_display()
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
	else if (display_type_ == MonoFontDisplayType::FixedRange) {
		// Use actual locale (%L) for formating
		value_str = QString("%L1").
			arg(value_, total_digits_, 'f', decimal_places_, QChar(' '));
	}
	else if (display_type_ == MonoFontDisplayType::AutoRangeWithSRDigits) {
		util::format_value_si(value_, total_digits_, sr_digits_, value_str,
			si_prefix);
	}
	else if (display_type_ == MonoFontDisplayType::AutoRange) {
		util::format_value_si_autoscale(value_, total_digits_, decimal_places_,
			value_str, si_prefix);
	}
	show_value(value_str);

	if (total_digits_changed_) {
		total_digits_changed_ = false;
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
