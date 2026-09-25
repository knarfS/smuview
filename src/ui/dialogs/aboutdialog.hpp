/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DIALOGS_ABOUTDIALOG_HPP
#define UI_DIALOGS_ABOUTDIALOG_HPP

#include <memory>

#include <QDialog>
#include <QTextBrowser>

#include "src/devices/basedevice.hpp"
#include "src/devicemanager.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
class BaseDevice;
}

namespace ui {
namespace dialogs {

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	AboutDialog(DeviceManager &device_manager,
		shared_ptr<sv::devices::BaseDevice> device,
		QWidget *parent = nullptr);

private:
	void setup_ui();
	QTextBrowser *get_about_page() const;
	QTextBrowser *get_device_page() const;
	QTextBrowser *get_versions_page() const;
	QTextBrowser *get_license_page() const;
	void copy_version_info();
	void open_manual();

	DeviceManager &device_manager_;
	shared_ptr<sv::devices::BaseDevice> device_;

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_ABOUTDIALOG_HPP
