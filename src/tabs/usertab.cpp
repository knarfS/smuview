/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#include "usertab.hpp"
#include "src/session.hpp"
#include "src/dialogs/addviewdialog.hpp"

namespace sv {
namespace tabs {

UserTab::UserTab(Session &session, QMainWindow *parent) :
	BaseTab(session, parent),
	action_add_control_view_(new QAction(this)),
	action_add_panel_view_(new QAction(this)),
	action_add_plot_view_(new QAction(this))
{
	setup_toolbar();
}

void UserTab::setup_toolbar()
{
	action_add_control_view_->setText(tr("Add &Control..."));
	action_add_control_view_->setIcon(
		QIcon::fromTheme("multimedia-volume-control",
						 QIcon(":/icons/multimedia-volume-control.png")));
	action_add_control_view_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
	connect(action_add_control_view_, SIGNAL(triggered(bool)),
			this, SLOT(on_action_add_control_view_triggered()));

	action_add_panel_view_->setText(tr("Add &Panel..."));
	action_add_panel_view_->setIcon(
		QIcon::fromTheme("video-display",
						 QIcon(":/icons/video-display.png")));
	action_add_panel_view_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
	connect(action_add_panel_view_, SIGNAL(triggered(bool)),
			this, SLOT(on_action_add_panel_view_triggered()));

	action_add_plot_view_->setText(tr("Add P&lot..."));
	action_add_plot_view_->setIcon(
		QIcon::fromTheme("office-chart-line",
						 QIcon(":/icons/office-chart-line.png")));
	action_add_plot_view_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
	connect(action_add_plot_view_, SIGNAL(triggered(bool)),
			this, SLOT(on_action_add_plot_view_triggered()));

	toolbar = new QToolBar("Device Toolbar");
	toolbar->addAction(action_add_control_view_);
	toolbar->addAction(action_add_panel_view_);
	toolbar->addAction(action_add_plot_view_);
	parent_->addToolBar(Qt::TopToolBarArea, toolbar);
}
void UserTab::on_action_add_control_view_triggered()
{
	dialogs::AddViewDialog dlg(session(), nullptr, 0);
	dlg.exec();

	for (auto view : dlg.views())
		add_view(view, Qt::TopDockWidgetArea);
}

void UserTab::on_action_add_panel_view_triggered()
{
	dialogs::AddViewDialog dlg(session(), nullptr, 1);
	dlg.exec();

	for (auto view : dlg.views())
		add_view(view, Qt::TopDockWidgetArea);
}

void UserTab::on_action_add_plot_view_triggered()
{
	dialogs::AddViewDialog dlg(session(), nullptr, 2);
	dlg.exec();

	for (auto view : dlg.views())
		add_view(view, Qt::BottomDockWidgetArea);
}

} // namespace tabs
} // namespace sv
