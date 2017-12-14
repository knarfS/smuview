/*
 * This file is part of the SmuView project.
 *
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

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "devicetab.hpp"
#include "src/session.hpp"
#include "src/data/analogdata.hpp"
#include "src/data/basesignal.hpp"
#include "src/dialogs/aboutdialog.hpp"
#include "src/dialogs/savedialog.hpp"

namespace sv {
namespace tabs {

DeviceTab::DeviceTab(Session &session,
		shared_ptr<devices::HardwareDevice> device, QMainWindow *parent) :
		BaseTab(session, parent),
	device_(device),
	action_open_(new QAction(this)),
	action_save_as_(new QAction(this)),
	action_add_control_view_(new QAction(this)),
	action_add_panel_view_(new QAction(this)),
	action_add_graph_view_(new QAction(this)),
	action_reset_data_(new QAction(this)),
	action_about_(new QAction(this))
{
	setup_toolbar();
}

void DeviceTab::clear_signals()
{
}

void DeviceTab::setup_toolbar()
{
	action_open_->setText(tr("&Open..."));
	action_open_->setIcon(
		QIcon::fromTheme("document-open",
		QIcon(":/icons/document-open.png")));
	action_open_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
	connect(action_open_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_open_triggered()));

	action_save_as_->setText(tr("&Save As..."));
	action_save_as_->setIcon(QIcon::fromTheme("document-save-as",
		QIcon(":/icons/document-save-as.png")));
	action_save_as_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	connect(action_save_as_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_save_as_triggered()));

	action_reset_data_->setText(tr("&Reset Data..."));
	action_reset_data_->setIcon(
		QIcon::fromTheme("view-refresh",
		QIcon(":/icons/view-refresh.png")));
	action_reset_data_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
	connect(action_reset_data_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_reset_data_triggered()));

	action_add_control_view_->setText(tr("Add &Control..."));
	action_add_control_view_->setIcon(
		QIcon::fromTheme("modem",
		QIcon(":/icons/modem.png")));
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

	action_add_graph_view_->setText(tr("Add &Graph..."));
	action_add_graph_view_->setIcon(
		QIcon::fromTheme("office-chart-line",
		QIcon(":/icons/office-chart-line.png")));
	action_add_graph_view_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	connect(action_add_graph_view_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_graph_view_triggered()));

	action_about_->setText(tr("&About..."));
	action_about_->setIcon(
		QIcon::fromTheme("help-about",
		QIcon(":/icons/help-about.png")));
	action_about_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
	connect(action_about_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_about_triggered()));

	toolbar = new QToolBar("Device Toolbar");
	toolbar->addAction(action_open_);
	toolbar->addAction(action_save_as_);
	toolbar->addSeparator();
	toolbar->addAction(action_reset_data_);
	toolbar->addSeparator();
	toolbar->addAction(action_add_control_view_);
	toolbar->addAction(action_add_panel_view_);
	toolbar->addAction(action_add_graph_view_);
	toolbar->addSeparator();
	toolbar->addAction(action_about_);
	parent_->addToolBar(Qt::TopToolBarArea, toolbar);
}

void DeviceTab::on_action_open_triggered()
{
}

void DeviceTab::on_action_save_as_triggered()
{
	dialogs::SaveDialog dlg(session(), device_->all_signals());
	dlg.exec();
}

void DeviceTab::on_action_add_control_view_triggered()
{
}

void DeviceTab::on_action_add_panel_view_triggered()
{
}

void DeviceTab::on_action_add_graph_view_triggered()
{
}

void DeviceTab::on_action_reset_data_triggered()
{
}

void DeviceTab::on_action_about_triggered()
{
	dialogs::AboutDialog dlg(this->session().device_manager());
	dlg.exec();
}


} // namespace tabs
} // namespace sv
