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

LcdDisplay::LcdDisplay(int digits, const QString unit, QWidget *parent) :
	QFrame(parent),
	digits_(digits),
	unit_(unit)
{
	setup_ui();

	lcdValue->setDigitCount(digits_);
	lcdUnit->setText(unit_);
}

void LcdDisplay::setup_ui()
{
	this->resize(210, 42);
	this->setFrameShape(QFrame::Box);

	QHBoxLayout *hLayout = new QHBoxLayout(this);
	QSpacerItem *horizontalSpacer = new QSpacerItem(
		40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	hLayout->addItem(horizontalSpacer);

	lcdValue = new QLCDNumber(this);
	QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(lcdValue->sizePolicy().hasHeightForWidth());
	lcdValue->setSizePolicy(sizePolicy);
	lcdValue->setMinimumSize(QSize(151, 50));
	QFont valueFont;
	valueFont.setPointSize(8);
	lcdValue->setFont(valueFont);
	lcdValue->setFrameShape(QFrame::NoFrame);
	lcdValue->setSmallDecimalPoint(true);
	lcdValue->setSegmentStyle(QLCDNumber::Flat);

	hLayout->addWidget(lcdValue);

	lcdUnit = new QLabel(this);
	QFont unitFont;
	unitFont.setPointSize(20);
	unitFont.setBold(true);
	unitFont.setWeight(75);
	lcdUnit->setFont(unitFont);
	lcdUnit->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

	hLayout->addWidget(lcdUnit);
}

void LcdDisplay::set_value(const double value)
{
	lcdValue->display(QString("%1").arg(value, 0, 'f', 3));
}

} // namespace widgets
} // namespace sv
