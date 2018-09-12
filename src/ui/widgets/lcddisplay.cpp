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

#include <limits>

#include <QDebug>
#include <QFont>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>

#include "lcddisplay.hpp"
#include "src/util.hpp"

namespace sv {
namespace ui {
namespace widgets {

LcdDisplay::LcdDisplay(
		uint digits, uint decimal_places, const bool auto_range,
		const QString unit, const QString unit_suffix, const QString extra_text,
		const bool small, QWidget *parent) :
	QFrame(parent),
	digits_(digits),
	decimal_places_(decimal_places),
	auto_range_(auto_range),
	unit_(unit),
	unit_si_prefix_(""),
	unit_suffix_(unit_suffix),
	update_unit_(true),
	extra_text_(extra_text),
	small_(small),
	value_(.0)
{
	setup_ui();
	reset_value();
}

void LcdDisplay::setup_ui()
{
	uint font_size_unit;
	uint font_size_extra;
	if (!small_) {
		height_ = 50;
		width_scale_factor_ = 30;
		font_size_unit = 18;;
		font_size_extra = 10;
	}
	else {
		height_ = 25;
		width_scale_factor_ = 15;
		font_size_unit = 10;
		font_size_extra = 7;
	}

	/*
	if (digits_ > 3) {
		// This is a workaround, b/c LCDNumber shows one digit less with
		// smallDecimalPoint == true && digits > 3
		digits_++;
	}

	// TODO: Are the number of digits displayed correctly?
	// TODO: Maybe there is a better way to draw the width proportional to the
	// height? This is maybe even dpi dependent?
	uint width = width_scale_factor * digits_;
	*/

	//this->setFrameShape(QFrame::Box);
	QSizePolicy layout_size_policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	layout_size_policy.setHorizontalStretch(0);
	layout_size_policy.setVerticalStretch(0);
	//layout_size_policy.setWidthForHeight(true); // TODO: maybe this could work somehow?
	this->setSizePolicy(layout_size_policy);

	QHBoxLayout *layout = new QHBoxLayout();

	lcd_value_ = new QLCDNumber();

	/*
	lcd_value_->setDigitCount(digits_);
	//lcd_value_->setMinimumSize(QSize(width, height));
	lcd_value_->setFixedSize(QSize(width, height));
	*/
	update_size();

	lcd_value_->setFrameShape(QFrame::NoFrame);
	//lcd_value_->setFrameShape(QFrame::Box);
	lcd_value_->setSmallDecimalPoint(true);
	lcd_value_->setSegmentStyle(QLCDNumber::Flat);
	layout->addWidget(lcd_value_);

	QVBoxLayout *text_layout = new QVBoxLayout();
	text_layout->addStretch(5);

	// Extra text (small)
	lcd_extra_ = new QLabel();
	QFont extra_font;
	extra_font.setPointSize(font_size_extra);
	//extra_font.setBold(true);
	//extra_font.setWeight(QFont::Bold);
	lcd_extra_->setFont(extra_font);
	lcd_extra_->setText(extra_text_);
	lcd_extra_->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
	text_layout->addWidget(lcd_extra_);

	// Unit
	lcd_unit_ = new QLabel();
	QFont unit_font;
	unit_font.setPointSize(font_size_unit);
	if (!small_) {
		unit_font.setBold(true);
		unit_font.setWeight(QFont::Bold);
	}
	lcd_unit_->setFont(unit_font);
	lcd_unit_->setText(QString("%1%2<small> %3</small>").
		arg(unit_si_prefix_).arg(unit_).arg(unit_suffix_));
	lcd_unit_->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
	text_layout->addWidget(lcd_unit_);

	layout->addLayout(text_layout);

	this->setLayout(layout);
}

void LcdDisplay::update_size()
{
	if (digits_ > 3) {
		// This is a workaround, b/c LCDNumber shows one digit less with
		// smallDecimalPoint == true && digits > 3
		digits_++;
	}

	// TODO: Are the number of digits displayed correctly?
	// TODO: Maybe there is a better way to draw the width proportional to the
	// height? This is maybe even dpi dependent?
	uint width = width_scale_factor_ * digits_;

	lcd_value_->setDigitCount(digits_);
	//lcd_value_->setMinimumSize(QSize(width, height_));
	lcd_value_->setFixedSize(QSize(width, height_));
}

double LcdDisplay::value() const
{
	return value_;
}

void LcdDisplay::set_value(const double value)
{
	value_ = value;
	update_display();
}


void LcdDisplay::set_unit(const QString unit)
{
	unit_ = unit;
	update_unit_ = true;
	update_display();
}

void LcdDisplay::set_unit_suffix(const QString unit_suffix)
{
	unit_suffix_ = unit_suffix;
	update_unit_ = true;
	update_display();
}

void LcdDisplay::set_extra_text(const QString extra_text)
{
	extra_text_ = extra_text;
	lcd_extra_->setText(extra_text_);
}

void LcdDisplay::set_digits(const uint digits, const uint decimal_places)
{
	digits_ = digits;
	decimal_places_ = decimal_places;
	update_size();
	update_display();
}

void LcdDisplay::reset_value()
{
	QString init_value("");
	for (uint i=0; i<digits_; i++)
		init_value.append("-");

	lcd_value_->display(init_value);
}

void LcdDisplay::update_display()
{
	QString value_str("");
	QString si_prefix("");

	if (value_ >= std::numeric_limits<double>::max() ||
			value_ == std::numeric_limits<double>::infinity()) {
		value_str = QString("OL");
	}
	else if (value_ <= std::numeric_limits<double>::lowest()) {
		value_str = QString("UL");
	}
	else if (!auto_range_) {
		value_str = QString("%1<small> %2</small>").
			arg(value_, digits_, 'f', decimal_places_, QChar(' ')).
			arg(unit_suffix_);
	}
	else {
		util::format_value_si(
			value_, digits_, decimal_places_, value_str, si_prefix);
	}

	lcd_value_->display(value_str);
	if (si_prefix != unit_si_prefix_ || update_unit_) {
		unit_si_prefix_ = si_prefix;
		QString unit_str = QString("%1%2<small> %3</small>").
			arg(unit_si_prefix_).arg(unit_).arg(unit_suffix_);
		lcd_unit_->setText(unit_str);
	}
}

} // namespace widgets
} // namespace ui
} // namespace sv
