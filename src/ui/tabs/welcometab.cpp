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

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include "welcometab.hpp"
#include "src/session.hpp"
#include <config.h>

namespace sv {
namespace ui {
namespace tabs {

WelcomeTab::WelcomeTab(Session &session, QWidget *parent) :
	BaseTab(session, parent)
{
	setup_ui();
}


string WelcomeTab::tab_id()
{
	return "welcometab";
}

QString WelcomeTab::tab_title()
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

	QString welcome("");
	welcome.
		append("<center>").
		append("<big>Welcome to <b>SmuView</b></big><br>").
		append("Multimeters, Power Supplies and Loads<br><br>").
		append("Version ").append(SV_VERSION_STRING).append("<br><br>").
		append("Copyright 2017-2020, Frank Stettner<br>").
		append("Lizenz: <a href=\"https://www.gnu.org/licenses/gpl.html\">GNU General Public License Version 3</a><br><br>").
		append("<a href=\"https://github.com/knarfS/smuview\">github.com/knarfS/smuview</a><br>").
		append("</center>");

	QLabel *welcome_label = new QLabel();
	welcome_label->setTextFormat(Qt::RichText);
	welcome_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	welcome_label->setOpenExternalLinks(true);
	welcome_label->setText(welcome);
	layout->addWidget(welcome_label);

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

} // namespace tabs
} // namespace ui
} // namespace sv
