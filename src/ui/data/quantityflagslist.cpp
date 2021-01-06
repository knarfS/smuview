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
#include <QListView>
#include <QListWidgetItem>
#include <QVariant>

#include "quantityflagslist.hpp"
#include "src/data/datautil.hpp"

Q_DECLARE_METATYPE(sv::data::QuantityFlag)

namespace sv {
namespace ui {
namespace data {

QuantityFlagsList::QuantityFlagsList(QWidget *parent) :
	QListWidget(parent)
{
	setup_ui();
}

void QuantityFlagsList::select_quantity_flags(
	const set<sv::data::QuantityFlag> &quantity_flags)
{
	for (int i = 0; i < this->count(); ++i) {
		QListWidgetItem* item = this->item(i);
		sv::data::QuantityFlag qf =
			item->data(Qt::UserRole).value<sv::data::QuantityFlag>();

		if (quantity_flags.count(qf) > 0)
			this->item(i)->setSelected(true);
		else
			this->item(i)->setSelected(false);
	}
}

set<sv::data::QuantityFlag> QuantityFlagsList::selected_quantity_flags()
{
	set<sv::data::QuantityFlag> flags;
	const auto items = this->selectedItems();
	for (const auto &item : items) {
		QVariant data = item->data(Qt::UserRole);
		if (data.isNull())
			continue;

		flags.insert(data.value<sv::data::QuantityFlag>());
	}
	return flags;
}

void QuantityFlagsList::setup_ui()
{
	this->setSelectionMode(QListView::MultiSelection);

	for (const auto &qf_name_pair : sv::data::datautil::get_quantity_flag_name_map()) {
		QListWidgetItem *item = new QListWidgetItem();
		item->setText(qf_name_pair.second);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setData(Qt::UserRole, QVariant::fromValue(qf_name_pair.first));
		this->addItem(item);
	}
}

} // namespace data
} // namespace ui
} // namespace sv

