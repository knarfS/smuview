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

#include <QComboBox>
#include <QDebug>
#include <QVariant>

#include "unitcombobox.hpp"
#include "src/data/datautil.hpp"

Q_DECLARE_METATYPE(sv::data::Unit)

namespace sv {
namespace widgets {

UnitComboBox::UnitComboBox(QWidget *parent) :
	QComboBox(parent)
{
	setup_ui();
}

data::Unit UnitComboBox::selected_unit()
{
	QVariant data = this->currentData();
	return data.value<data::Unit>();
}

void UnitComboBox::setup_ui()
{
	for (auto u_name_pair : data::datautil::get_unit_name_map()) {
		this->addItem(
			u_name_pair.second, QVariant::fromValue(u_name_pair.first));
	}
}

} // namespace widgets
} // namespace sv

