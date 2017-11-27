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

#include <QVBoxLayout>

#include "lcddisplay.hpp"

namespace sv {
namespace widgets {

LcdDisplay::LcdDisplay(const uint digits, const QString unit,
		const QString extra_text, const bool small, QWidget *parent) :
	QFrame(parent),
	digits_(digits),
	unit_(unit),
	extra_text_(extra_text)
{
	if (!small) {
		height = 50;
		width_scale_factor_ = 30;
		font_size_unit_ = 18;;
		font_size_extra_ = 10;
	}
	else {
		height = 25;
		width_scale_factor_ = 15;
		font_size_unit_ = 10;
		font_size_extra_ = 7;
	}

	setup_ui();
	reset_value();
}

void LcdDisplay::setup_ui()
{
	// TODO: Are ne number of digits displayed correctly?
	// TODO: Maybe there is a better way to draw the width proportional to the
	// height? This is maybe even dpi dependent?
	uint width = width_scale_factor_ * digits_;

	//this->setFrameShape(QFrame::Box);
	QSizePolicy layoutSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	layoutSizePolicy.setHorizontalStretch(0);
	layoutSizePolicy.setVerticalStretch(0);
	//lcdSizePolicy.setWidthForHeight(true); // TODO: maybe this could work somehow?
	this->setSizePolicy(layoutSizePolicy);

	QHBoxLayout *layout = new QHBoxLayout();

	lcdValue = new QLCDNumber();
	lcdValue->setDigitCount(digits_);
	//lcdValue->setMinimumSize(QSize(width, height));
	lcdValue->setFixedSize(QSize(width, height));

	lcdValue->setFrameShape(QFrame::NoFrame);
	//lcdValue->setFrameShape(QFrame::Box);
	lcdValue->setSmallDecimalPoint(true);
	lcdValue->setSegmentStyle(QLCDNumber::Flat);
	layout->addWidget(lcdValue);

	QVBoxLayout *textLayout = new QVBoxLayout();
	textLayout->addStretch(5);

	// Extra text
	lcdExtra = new QLabel();
	QFont extraFont;
	extraFont.setPointSize(font_size_extra_);
	//extraFont.setBold(true);
	//extraFont.setWeight(QFont::Bold);
	lcdExtra->setFont(extraFont);
	lcdExtra->setText(extra_text_);
	lcdExtra->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
	textLayout->addWidget(lcdExtra);

	// Unit
	lcdUnit = new QLabel();
	QFont unitFont;
	unitFont.setPointSize(font_size_unit_);
	unitFont.setBold(true);
	unitFont.setWeight(QFont::Bold);
	lcdUnit->setFont(unitFont);
	lcdUnit->setText(unit_);
	lcdUnit->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
	textLayout->addWidget(lcdUnit);

	layout->addLayout(textLayout);

	this->setLayout(layout);
}

void LcdDisplay::set_value(const double value)
{
	QString str_value;
	if (value == std::numeric_limits<double>::max())
		str_value = QString("OL");
	else
		str_value = QString("%1").arg(value, 0, 'f', 3);

	lcdValue->display(str_value);
}


void LcdDisplay::set_unit(const QString unit)
{
	unit_ = unit;
	lcdUnit->setText(unit_);
}

void LcdDisplay::reset_value()
{
	QString init_value("");
	for (uint i=0; i<digits_; i++)
		init_value.append("-");

	lcdValue->display(init_value);
}

} // namespace widgets
} // namespace sv
