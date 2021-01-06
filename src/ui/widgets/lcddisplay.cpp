/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QFont>
#include <QLCDNumber>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>

#include "lcddisplay.hpp"
#include "src/ui/widgets/valuedisplay.hpp"

namespace sv {
namespace ui {
namespace widgets {

LcdDisplay::LcdDisplay(
		int digits, int decimal_places, const bool auto_range,
		const QString &unit, const QString &unit_suffix,
		const QString &extra_text, const bool small, QWidget *parent) :
	ValueDisplay(digits, decimal_places, auto_range, unit, unit_suffix,
		extra_text, small, parent)
{
	LcdDisplay::setup_ui();
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

	//this->setFrameShape(QFrame::Box);
	QSizePolicy layout_size_policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	layout_size_policy.setHorizontalStretch(0);
	layout_size_policy.setVerticalStretch(0);
	this->setSizePolicy(layout_size_policy);

	QHBoxLayout *layout = new QHBoxLayout();

	value_lcd_ = new QLCDNumber();
	value_lcd_->setFrameShape(QFrame::NoFrame);
	//value_lcd_->setFrameShape(QFrame::Box);
	value_lcd_->setSmallDecimalPoint(true);
	value_lcd_->setSegmentStyle(QLCDNumber::Flat);
	layout->addWidget(value_lcd_);

	QVBoxLayout *text_layout = new QVBoxLayout();
	text_layout->addStretch(5);

	// Extra text (small)
	extra_label_ = new QLabel();
	QFont extra_font;
	extra_font.setPointSize(font_size_extra);
	extra_label_->setFont(extra_font);
	extra_label_->setText(extra_text_);
	extra_label_->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
	text_layout->addWidget(extra_label_);

	// Unit
	unit_label_ = new QLabel();
	QFont unit_font;
	unit_font.setPointSize(font_size_unit);
	if (!small_) {
		unit_font.setBold(true);
		unit_font.setWeight(QFont::Bold);
	}
	unit_label_->setFont(unit_font);
	unit_label_->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
	text_layout->addWidget(unit_label_);

	layout->addLayout(text_layout);

	this->setLayout(layout);
}

void LcdDisplay::update_value_widget_dimensions()
{
	if (digits_ > 3) {
		// This is a workaround, b/c LCDNumber shows one digit less with
		// smallDecimalPoint == true && digits > 3
		digits_++;
	}

	// TODO: Are the number of digits displayed correctly?
	// TODO: Maybe there is a better way to draw the width proportional to the
	//       height? This is maybe even dpi dependent?
	uint width = width_scale_factor_ * digits_;

	value_lcd_->setDigitCount(digits_);
	value_lcd_->setFixedSize(QSize(width, height_));
}

void LcdDisplay::update_extra_widget_dimensions()
{
}

void LcdDisplay::update_unit_widget_dimensions()
{
	QString str;
	if (auto_range_) {
		// 'm' is the widest character for non monospace fonts
		str.append("m");
	}
	str.append(unit_);
	if (!unit_suffix_.isEmpty()) {
		str.append(" ").append(unit_suffix_);
	}
	QFontMetrics metrics = unit_label_->fontMetrics();
	unit_label_->setFixedWidth(metrics.boundingRect(str).width());
}

void LcdDisplay::show_value(const QString &value)
{
	value_lcd_->display(value);
}

void LcdDisplay::show_extra_text(const QString &extra_text)
{
	extra_label_->setText(extra_text);
}

void LcdDisplay::show_unit(const QString &unit)
{
	unit_label_->setText(unit);
}

} // namespace widgets
} // namespace ui
} // namespace sv
