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

#include <map>
#include <memory>

#include <QComboBox>
#include <QDebug>
#include <QVariant>

#include "configurablecombobox.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"

using std::shared_ptr;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace devices {

ConfigurableComboBox::ConfigurableComboBox(
		shared_ptr<sv::devices::BaseDevice> device,
		QWidget *parent) :
	QComboBox(parent),
	device_(device)
{
	setup_ui();
}

void ConfigurableComboBox::select_configurable(
	shared_ptr<sv::devices::Configurable> configurable)
{
	for (int i = 0; i < this->count(); ++i) {
		QVariant data = this->itemData(i, Qt::UserRole);
		auto item_config = data.value<shared_ptr<sv::devices::Configurable>>();
		if (item_config == configurable) {
			this->setCurrentIndex(i);
			break;
		}
	}
}

shared_ptr<sv::devices::Configurable>
	ConfigurableComboBox::selected_configurable() const
{
	QVariant data = this->currentData();
	return data.value<shared_ptr<sv::devices::Configurable>>();
}

void ConfigurableComboBox::setup_ui()
{
	this->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	this->fill_configurables();
}

void ConfigurableComboBox::fill_configurables()
{
	this->clear();

	if (device_ == nullptr)
		return;

	for (const auto &c_pair : device_->configurable_map()) {
		// Only show configurables that either are getable, setable or listable.
		auto configurable = c_pair.second;
		if (!configurable->is_controllable())
			continue;

		this->addItem(
			configurable->display_name(),
			QVariant::fromValue(configurable));
	}
}

void ConfigurableComboBox::change_device(
	shared_ptr<sv::devices::BaseDevice> device)
{
	device_ = device;
	this->fill_configurables();
}

} // namespace devices
} // namespace ui
} // namespace sv
