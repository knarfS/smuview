/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef WIDGETS_UNITCOMBOBOX_HPP
#define WIDGETS_UNITCOMBOBOX_HPP

#include <QComboBox>
#include <QWidget>

namespace sigrok {
class Unit;
}

namespace sv {

namespace widgets {

class UnitComboBox : public QComboBox
{
    Q_OBJECT

public:
	UnitComboBox(QWidget *parent = nullptr);

	const sigrok::Unit *selected_sr_unit();

private:
	void setup_ui();

};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_UNITCOMBOBOX_HPP

