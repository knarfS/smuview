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

#include "powerpanel.hpp"

namespace sv {
namespace widgets {

PowerPanel::PowerPanel(QWidget *parent) :
	QWidget(parent)
{
	setup_ui();
}

void PowerPanel::setup_ui()
{
	QVBoxLayout *getValuesVLayout = new QVBoxLayout(this);

	voltageDisplay = new widgets::LcdDisplay(5, "V", this);
	currentDisplay = new widgets::LcdDisplay(5, "A", this);
	resistanceDisplay = new widgets::LcdDisplay(5, QString::fromUtf8("\u2126"), this);
	powerDisplay = new widgets::LcdDisplay(5, "W", this);
	ampHourDisplay = new widgets::LcdDisplay(5, "Ah", this);
	wattHourDisplay = new widgets::LcdDisplay(5, "Wh", this);

	QHBoxLayout *getUpperHLayout = new QHBoxLayout(this);
	getUpperHLayout->addWidget(voltageDisplay);
	getUpperHLayout->addWidget(powerDisplay);
	getUpperHLayout->addWidget(ampHourDisplay);

	QHBoxLayout *getLowerHLayout = new QHBoxLayout(this);
	getLowerHLayout->addWidget(currentDisplay);
	getLowerHLayout->addWidget(resistanceDisplay);
	getLowerHLayout->addWidget(wattHourDisplay);

	getValuesVLayout->addItem(getUpperHLayout);
	getValuesVLayout->addItem(getLowerHLayout);
	getValuesVLayout->addStretch(4);
}

} // namespace widgets
} // namespace sv
