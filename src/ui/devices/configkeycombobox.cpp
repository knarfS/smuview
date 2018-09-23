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

#include <QDebug>
#include <QVariant>

#include "configkeycombobox.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/properties/baseproperty.hpp"

Q_DECLARE_METATYPE(sv::devices::ConfigKey)

namespace sv {
namespace ui {
namespace devices {

ConfigKeyComboBox::ConfigKeyComboBox(
		shared_ptr<sv::devices::Configurable> configurable,
		const bool show_getable, const bool show_setable,
		const bool show_listable,  QWidget *parent) :
	QComboBox(parent),
	configurable_(configurable),
	show_getable_(show_getable),
	show_setable_(show_setable),
	show_listable_(show_listable)
{
	setup_ui();
}

void ConfigKeyComboBox::select_config_key(sv::devices::ConfigKey config_key)
{
	for (int i = 0; i < this->count(); ++i) {
		QVariant data = this->itemData(i, Qt::UserRole);
		auto item_ck = data.value<sv::devices::ConfigKey>();
		if (item_ck == config_key) {
			this->setCurrentIndex(i);
			break;
		}
	}
}

sv::devices::ConfigKey ConfigKeyComboBox::selected_config_key() const
{
	QVariant data = this->currentData();
	if (!data.isValid())
		return sv::devices::ConfigKey::Unknown;
	return data.value<sv::devices::ConfigKey>();
}

void ConfigKeyComboBox::setup_ui()
{
	this->clear();

	if (configurable_ == nullptr)
		return;

	for (auto prop : configurable_->properties()) {
		if (!(show_getable_ && prop.second->is_getable()) &&
				!(show_setable_ && prop.second->is_setable()) &&
				!(show_listable_ && prop.second->is_listable()))
			continue;

		this->addItem(
			sv::devices::deviceutil::format_config_key(prop.first),
			QVariant::fromValue(prop.first));
	}
}

void ConfigKeyComboBox::change_configurable(
	shared_ptr<sv::devices::Configurable> configurable)
{
	configurable_ = configurable;
	this->setup_ui();
}

} // namespace devices
} // namespace ui
} // namespace sv
