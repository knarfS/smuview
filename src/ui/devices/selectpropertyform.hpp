/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DEVICES_SELECTPROPERTYFORM_HPP
#define UI_DEVICES_SELECTPROPERTYFORM_HPP

#include <memory>
#include <set>

#include <QFormLayout>

#include "src/devices/deviceutil.hpp"

using std::set;
using std::shared_ptr;

namespace sv {

class Session;

namespace data {
namespace properties {
class BaseProperty;
}
}
namespace devices {
class BaseDevice;
class Configurable;
}

namespace ui {
namespace devices {

class ConfigKeyComboBox;
class ConfigurableComboBox;
class DeviceComboBox;

class SelectPropertyForm : public QFormLayout
{
	Q_OBJECT

public:
	explicit SelectPropertyForm(const Session &session,
		QWidget *parent = nullptr);

	void filter_config_keys(const set<sv::data::DataType> &data_types);
	void select_device(shared_ptr<sv::devices::BaseDevice> device);
	void select_configurable(shared_ptr<sv::devices::Configurable> configurable);
	void select_config_key(sv::devices::ConfigKey config_key);
	shared_ptr<sv::devices::BaseDevice> selected_device() const;
	shared_ptr<sv::devices::Configurable> selected_configurable() const;
	shared_ptr<sv::data::properties::BaseProperty> selected_property() const;
	sv::devices::ConfigKey selected_config_key() const;


private:
	void setup_ui();
	void connect_signals();

	const Session &session_;

	DeviceComboBox *device_box_;
	ConfigurableComboBox *configurable_box_;
	ConfigKeyComboBox *config_key_box_;

private Q_SLOTS:
	void on_device_changed();
	void on_configurable_changed();

};

} // namespace devices
} // namespace ui
} // namespace sv

#endif // UI_DEVICES_SELECTPROPERTYFORM_HPP
