/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012-2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DIALOGS_CONNECT_HPP
#define DIALOGS_CONNECT_HPP

#include <memory>

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "src/devices/hardwaredevice.hpp"

using std::shared_ptr;

namespace sigrok {
class Driver;
}

Q_DECLARE_METATYPE(shared_ptr<sigrok::Driver>);
Q_DECLARE_METATYPE(shared_ptr<sv::devices::HardwareDevice>);

namespace sv {

class DeviceManager;

namespace devices {
class HardwareDevice;
}

namespace dialogs {

class Connect : public QDialog
{
	Q_OBJECT

public:
	Connect(QWidget *parent, sv::DeviceManager &device_manager);

	shared_ptr<devices::HardwareDevice> get_selected_device() const;

private:
	void populate_drivers();

	void populate_serials(shared_ptr<sigrok::Driver> driver);

	void check_available_libs();

	void unset_connection();

private Q_SLOTS:
	void driver_selected(int index);

	void serial_toggled(bool checked);
	void tcp_toggled(bool checked);
	void gpib_toggled(bool checked);

	void scan_pressed();

private:
	sv::DeviceManager &device_manager_;

	bool gpib_avialable_;

	QVBoxLayout layout_;

	QWidget form_;
	QFormLayout form_layout_;

	QComboBox drivers_;

	QComboBox serial_devices_;

	QWidget *tcp_config_;
	QLineEdit *tcp_host_;
	QSpinBox *tcp_port_;
	QCheckBox *tcp_use_vxi_;

	QLineEdit *gpib_libgpib_name_;

	QPushButton scan_button_;
	QListWidget device_list_;

	QDialogButtonBox button_box_;
	
};

} // namespace dialogs
} // namespace sv

#endif // DIALOGS_CONNECT_HPP
