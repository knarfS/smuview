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

#ifndef WIDGETS_QUANTITYFLAGSLIST_HPP
#define WIDGETS_QUANTITYFLAGSLIST_HPP

#include <vector>

#include <QListWidget>
#include <QWidget>

using std::vector;

namespace sigrok {
class QuantityFlag;
}

namespace sv {

namespace widgets {

class QuantityFlagsList : public QListWidget
{
    Q_OBJECT

public:
	QuantityFlagsList(QWidget *parent = nullptr);

	vector<const sigrok::QuantityFlag *> selected_sr_quantity_flags();

private:
	void setup_ui();

};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_QUANTITYFLAGSLIST_HPP

