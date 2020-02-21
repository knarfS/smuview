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

#include <memory>
#include <set>

#include <QComboBox>
#include <QDebug>
#include <QVariant>

#include "configkeycombobox.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/configurable.hpp"

using std::set;
using std::shared_ptr;

Q_DECLARE_METATYPE(sv::devices::ConfigKey)

namespace sv {
namespace ui {
namespace devices {

ConfigKeyComboBox::ConfigKeyComboBox(
		shared_ptr<sv::devices::Configurable> configurable,
		QWidget *parent) :
	QComboBox(parent),
	configurable_(configurable)
{
	setup_ui();
}

void ConfigKeyComboBox::filter_config_keys(set<sv::data::DataType> data_types)
{
	filter_data_types_ = data_types;
	this->fill_config_keys();
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
	return data.value<sv::devices::ConfigKey>();
}

void ConfigKeyComboBox::setup_ui()
{
	this->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	this->fill_config_keys();
}

void ConfigKeyComboBox::fill_config_keys()
{
	this->clear();

	if (configurable_ == nullptr)
		return;

	// TODO: Filter for getable, setable, listable
	for (const auto &config_key : configurable_->setable_configs()) {
		sv::data::DataType dt =
			sv::devices::deviceutil::get_data_type_for_config_key(config_key);
		if (filter_data_types_.empty() || filter_data_types_.count(dt)) {
			this->addItem(
				sv::devices::deviceutil::format_config_key(config_key),
				QVariant::fromValue(config_key));
		}
	}
}

void ConfigKeyComboBox::change_configurable(
	shared_ptr<sv::devices::Configurable> configurable)
{
	configurable_ = configurable;
	this->fill_config_keys();
}

} // namespace devices
} // namespace ui
} // namespace sv
