/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2022 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "welcometab.hpp"
#include "src/config.h"
#include "src/mainwindow.hpp"
#include "src/session.hpp"
#include "src/ui/dialogs/connectdialog.hpp"
#include "src/ui/tabs/basetab.hpp"

namespace sv {
namespace ui {
namespace tabs {

WelcomeTab::WelcomeTab(Session &session, QWidget *parent) :
	BaseTab(session, parent)
{
	id_ = "welcometab:0";
	settings_id_ = "welcometab:0";

	setup_ui();
}


QString WelcomeTab::title()
{
	return tr("Welcome");
}

bool WelcomeTab::request_close()
{
	return true;
}

void WelcomeTab::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	layout->addStretch();

	QLabel *smuview_icon = new QLabel();
	smuview_icon->setPixmap(QIcon(":/icons/smuview.svg").pixmap(72, 72));
	layout->addWidget(smuview_icon, 0, Qt::AlignHCenter);

	layout->addSpacing(20);

	QString welcome("");
	welcome
		.append("<center>")
		.append(tr("<big>Welcome to <b>SmuView</b></big>"))
		.append("<br>")
		.append(tr("Multimeters, Power Supplies, Loads and more"))
		.append("<br>")
		.append("<a href=\"https://github.com/knarfS/smuview\">github.com/knarfS/smuview</a><br><br>")
		.append("Version ").append(SV_VERSION_STRING).append("<br><br>")
		.append("Copyright 2017-2026, Frank Stettner<br>")
		.append(tr("License: <a href=\"https://www.gnu.org/licenses/gpl.html\">GNU General Public License Version 3</a>"))
		.append("</center>");

	QLabel *welcome_label = new QLabel();
	welcome_label->setTextFormat(Qt::RichText);
	welcome_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	welcome_label->setOpenExternalLinks(true);
	welcome_label->setText(welcome);
	layout->addWidget(welcome_label);

	layout->addSpacing(30);

	QHBoxLayout *button_layout = new QHBoxLayout();
	button_layout->addStretch(2);
	QPushButton *add_device_button = new QPushButton();
	add_device_button->setText(tr("Connect new device"));
	connect(add_device_button, &QPushButton::pressed,
		this, &WelcomeTab::on_add_device);
	button_layout->addWidget(add_device_button, 1);
	QPushButton *open_manual_button = new QPushButton();
	open_manual_button->setText(tr("Open SmuView manual"));
	connect(open_manual_button, &QPushButton::pressed,
		this, &WelcomeTab::on_open_manual);
	button_layout->addWidget(open_manual_button, 1);
	button_layout->addStretch(2);
	layout->addLayout(button_layout);

	layout->addStretch();

	// Show the central widget of the tab (hidden by BaseTab)
	this->centralWidget()->show();
	this->centralWidget()->setLayout(layout);
}

void WelcomeTab::restore_settings()
{
}

void WelcomeTab::save_settings() const
{
}

void WelcomeTab::on_add_device()
{
	dialogs::ConnectDialog dlg(session().device_manager(), this);
	if (!dlg.exec())
		return;

	auto device = dlg.get_selected_device();
	// NOTE: add_device() must be called, before the device tab
	//       tries to access the device (device is not opend yet).
	session().add_device(device);
	session().main_window()->add_device_tab(device);
}

void WelcomeTab::on_open_manual()
{
	QUrl manual_url("https://knarfs.github.io/doc/smuview/continuous/manual.html");
	QDesktopServices::openUrl(manual_url);
}

} // namespace tabs
} // namespace ui
} // namespace sv
