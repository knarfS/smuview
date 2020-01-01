/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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
namespace ui {
namespace data {

UnitComboBox::UnitComboBox(QWidget *parent) :
	QComboBox(parent)
{
	setup_ui();
}

void UnitComboBox::select_unit(sv::data::Unit unit)
{
	QString name = sv::data::datautil::format_unit(unit);
	this->setCurrentText(name);
}

sv::data::Unit UnitComboBox::selected_unit()
{
	QVariant data = this->currentData();
	return data.value<sv::data::Unit>();
}

void UnitComboBox::setup_ui()
{
	for (const auto &u_name_pair : sv::data::datautil::get_unit_name_map()) {
		this->addItem(
			u_name_pair.second, QVariant::fromValue(u_name_pair.first));
	}
}

} // namespace data
} // namespace ui
} // namespace sv

