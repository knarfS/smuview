/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QVariant>

#include "quantitycombobox.hpp"
#include "src/data/datautil.hpp"

Q_DECLARE_METATYPE(sv::data::Quantity)

namespace sv {
namespace ui {
namespace data {


QuantityComboBox::QuantityComboBox(QWidget *parent) :
	QComboBox(parent)
{
	setup_ui();
}

void QuantityComboBox::select_quantity(sv::data::Quantity quantity)
{
	QString name = sv::data::datautil::format_quantity(quantity);
	this->setCurrentText(name);
}

sv::data::Quantity QuantityComboBox::selected_quantity()
{
	QVariant data = this->currentData();
	return data.value<sv::data::Quantity>();
}

void QuantityComboBox::setup_ui()
{
	for (const auto &q_name_pair : sv::data::datautil::get_quantity_name_map()) {
		this->addItem(
			q_name_pair.second, QVariant::fromValue(q_name_pair.first));
	}
}

} // namespace data
} // namespace ui
} // namespace sv
