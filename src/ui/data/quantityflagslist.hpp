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

#ifndef UI_DATA_QUANTITYFLAGSLIST_HPP
#define UI_DATA_QUANTITYFLAGSLIST_HPP

#include <set>

#include <QListWidget>
#include <QWidget>

#include "src/data/datautil.hpp"

using std::set;

namespace sv {
namespace ui {
namespace data {

class QuantityFlagsList : public QListWidget
{
	Q_OBJECT

public:
	QuantityFlagsList(QWidget *parent = nullptr);

	set<sv::data::QuantityFlag> selected_quantity_flags();

private:
	void setup_ui();

};

} // namespace data
} // namespace ui
} // namespace sv

#endif // UI_DATA_QUANTITYFLAGSLIST_HPP

