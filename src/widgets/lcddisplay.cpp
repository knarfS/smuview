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

LcdDisplay::LcdDisplay(const uint digits, const QString unit, QWidget *parent) :
	QFrame(parent),
	digits_(digits),
	unit_(unit)
{
	setup_ui();
	reset_value();
}

void LcdDisplay::setup_ui()
{
	this->resize(210, 42);
	this->setFrameShape(QFrame::Box);

	QHBoxLayout *hLayout = new QHBoxLayout(this);
	hLayout->addStretch(5);

	lcdValue = new QLCDNumber(this);
	lcdValue->setDigitCount(digits_);
	QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	//sizePolicy.setHeightForWidth(lcdValue->sizePolicy().hasHeightForWidth());
	lcdValue->setSizePolicy(sizePolicy);
	lcdValue->setMinimumSize(QSize(151, 50));
	lcdValue->setFrameShape(QFrame::NoFrame);
	//lcdValue->setFrameShape(QFrame::Box);
	lcdValue->setSmallDecimalPoint(true);
	lcdValue->setSegmentStyle(QLCDNumber::Flat);

	hLayout->addWidget(lcdValue);

	lcdUnit = new QLabel(this);
	QFont unitFont;
	unitFont.setPointSize(18);
	unitFont.setBold(true);
	unitFont.setWeight(75);
	lcdUnit->setFont(unitFont);
	lcdUnit->setText(unit_);
	lcdUnit->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

	hLayout->addWidget(lcdUnit);
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
