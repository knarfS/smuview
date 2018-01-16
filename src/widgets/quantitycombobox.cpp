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

#include "quantitycombobox.hpp"
#include "src/util.hpp"

using std::dynamic_pointer_cast;
using std::static_pointer_cast;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace widgets {


QuantityComboBox::QuantityComboBox(const Session &session, QWidget *parent) :
	QComboBox(parent),
	session_(session)
{
	setup_ui();
}

shared_ptr<const sigrok::Quantity *> QuantityComboBox::selected_sr_quantity()
{
	return nullptr;
}

void QuantityComboBox::setup_ui()
{
	for (auto q_name_pair : util::get_quantity_name_map()) {
		this->addItem(
			q_name_pair.second, QVariant::fromValue(q_name_pair.second));
	}
}

} // namespace widgets
} // namespace sv

