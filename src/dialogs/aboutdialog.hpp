/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Soeren Apel <soeren@apelpie.net>
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

#ifndef DIALOGS_ABOUTDIALOG_HPP
#define DIALOGS_ABOUTDIALOG_HPP

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>

#include "src/devicemanager.hpp"

namespace sv {

namespace devices {
class HardwareDevice;
}

namespace dialogs {

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	AboutDialog(DeviceManager &device_manager,
		shared_ptr<devices::HardwareDevice> device,
		QWidget *parent = nullptr);

private:
	void create_pages();
	QWidget *get_about_page(QWidget *parent) const;
	QWidget *get_device_page(QWidget *parent) const;

	DeviceManager &device_manager_;
	shared_ptr<devices::HardwareDevice> device_;

	QListWidget *page_list;
	QStackedWidget *pages;

private Q_SLOTS:
	void on_page_changed(QListWidgetItem *current, QListWidgetItem *previous);

};

} // namespace dialogs
} // namespace sv

#endif // DIALOGS_ABOUTDIALOG_HPP
