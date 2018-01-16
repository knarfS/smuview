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

#include <utility>

#include <QDebug>

#include "unitcombobox.hpp"
#include "src/util.hpp"

using std::dynamic_pointer_cast;
using std::static_pointer_cast;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace widgets {


UnitComboBox::UnitComboBox(const Session &session, QWidget *parent) :
	QComboBox(parent),
	session_(session)
{
	setup_ui();
}

shared_ptr<const sigrok::Unit *> UnitComboBox::selected_sr_unit()
{
	return nullptr;
}

void UnitComboBox::setup_ui()
{
	for (auto u_name_pair : util::get_unit_name_map()) {
		this->addItem(
			u_name_pair.second, QVariant::fromValue(u_name_pair.second));
	}
}

} // namespace widgets
} // namespace sv

