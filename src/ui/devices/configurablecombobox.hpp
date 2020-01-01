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

#ifndef UI_DEVICES_CONFIGURABLECOMBOBOX_HPP
#define UI_DEVICES_CONFIGURABLECOMBOBOX_HPP

#include <memory>

#include <QComboBox>
#include <QWidget>

using std::shared_ptr;

namespace sv {

namespace devices {
class BaseDevice;
class Configurable;
}

namespace ui {
namespace devices {

class ConfigurableComboBox : public QComboBox
{
	Q_OBJECT

public:
	ConfigurableComboBox(
		shared_ptr<sv::devices::BaseDevice> device,
		QWidget *parent = nullptr);

	void select_configurable(shared_ptr<sv::devices::Configurable>);
	shared_ptr<sv::devices::Configurable> selected_configurable() const;

private:
	void setup_ui();
	void fill_configurables();

	shared_ptr<sv::devices::BaseDevice> device_;

public Q_SLOTS:
	void change_device(shared_ptr<sv::devices::BaseDevice>);

};

} // namespace devices
} // namespace ui
} // namespace sv

#endif // UI_DEVICES_CONFIGURABLECOMBOBOX_HPP
