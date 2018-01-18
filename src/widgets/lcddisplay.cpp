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

#include <QDebug>
#include <QFont>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>

#include "lcddisplay.hpp"
#include "src/util.hpp"

namespace sv {
namespace widgets {

LcdDisplay::LcdDisplay(
		int digits, int decimal_places, const bool auto_range,
		const QString unit, const QString extra_text, const bool small,
		QWidget *parent) :
	QFrame(parent),
	digits_(digits),
	decimal_places_(decimal_places),
	auto_range_(auto_range),
	unit_(unit),
	unit_si_prefix_(""),
	update_unit_(true),
	extra_text_(extra_text),
	small_(small)
{
	if (digits_ > 3)
		// This is a workaround, b/c LCDNumber shows one digit less with
		// smallDecimalPoint == true && digits > 3
		digits_++;

	setup_ui();
	reset_value();
}

void LcdDisplay::setup_ui()
{
	uint height;
	uint width_scale_factor;
	uint font_size_unit;
	uint font_size_extra;
	if (!small_) {
		height = 50;
		width_scale_factor = 30;
		font_size_unit = 18;;
		font_size_extra = 10;
	}
	else {
		height = 25;
		width_scale_factor = 15;
		font_size_unit = 10;
		font_size_extra = 7;
	}
	// TODO: Are the number of digits displayed correctly?
	// TODO: Maybe there is a better way to draw the width proportional to the
	// height? This is maybe even dpi dependent?
	uint width = width_scale_factor * digits_;

	//this->setFrameShape(QFrame::Box);
	QSizePolicy layoutSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	layoutSizePolicy.setHorizontalStretch(0);
	layoutSizePolicy.setVerticalStretch(0);
	//lcdSizePolicy.setWidthForHeight(true); // TODO: maybe this could work somehow?
	this->setSizePolicy(layoutSizePolicy);

	QHBoxLayout *layout = new QHBoxLayout();

	lcdValue_ = new QLCDNumber();
	lcdValue_->setDigitCount(digits_);
	//lcdValue_->setMinimumSize(QSize(width, height));
	lcdValue_->setFixedSize(QSize(width, height));

	lcdValue_->setFrameShape(QFrame::NoFrame);
	//lcdValue_->setFrameShape(QFrame::Box);
	lcdValue_->setSmallDecimalPoint(true);
	lcdValue_->setSegmentStyle(QLCDNumber::Flat);
	layout->addWidget(lcdValue_);

	QVBoxLayout *textLayout = new QVBoxLayout();
	textLayout->addStretch(5);

	// Extra text (small)
	lcdExtra_ = new QLabel();
	QFont extraFont;
	extraFont.setPointSize(font_size_extra);
	//extraFont.setBold(true);
	//extraFont.setWeight(QFont::Bold);
	lcdExtra_->setFont(extraFont);
	lcdExtra_->setText(extra_text_);
	lcdExtra_->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
	textLayout->addWidget(lcdExtra_);

	// Unit
	lcdUnit_ = new QLabel();
	QFont unitFont;
	unitFont.setPointSize(font_size_unit);
	if (!small_) {
		unitFont.setBold(true);
		unitFont.setWeight(QFont::Bold);
	}
	lcdUnit_->setFont(unitFont);
	lcdUnit_->setText(QString("%1%2").arg(unit_si_prefix_).arg(unit_));
	lcdUnit_->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
	textLayout->addWidget(lcdUnit_);

	layout->addLayout(textLayout);

	this->setLayout(layout);
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

void LcdDisplay::set_digits(const int digits)
{
	digits_ = digits;
	update_display();
}

void LcdDisplay::set_decimal_places(const int decimal_places)
{
	decimal_places_ = decimal_places;
	update_display();
}

void LcdDisplay::reset_value()
{
	QString init_value("");
	for (int i=0; i<digits_; i++)
		init_value.append("-");

	lcdValue_->display(init_value);
}

void LcdDisplay::update_display()
{
	QString value_str;
	if (value_ >= std::numeric_limits<double>::max())
		value_str = QString("OL");
	else if (value_ <= std::numeric_limits<double>::lowest())
		value_str = QString("UL");
	else if (!auto_range_)
		value_str = QString("%1").
			arg(value_, digits_, 'f', decimal_places_, QChar(' '));
	else {
		QString si_prefix;
		util::format_value_si(
			value_, digits_, decimal_places_, value_str, si_prefix);
		if (si_prefix != unit_si_prefix_ || update_unit_) {
			unit_si_prefix_ = si_prefix;
			QString text = QString("%1%2").arg(unit_si_prefix_).arg(unit_);
			lcdUnit_->setText(text);
		}
	}
	lcdValue_->display(value_str);
}

} // namespace widgets
} // namespace sv
