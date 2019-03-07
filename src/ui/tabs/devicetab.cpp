/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QToolButton>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "devicetab.hpp"
#include "src/session.hpp"
#include "src/channels/userchannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/dialogs/aboutdialog.hpp"
#include "src/ui/dialogs/addmathchanneldialog.hpp"
#include "src/ui/dialogs/addviewdialog.hpp"
#include "src/ui/dialogs/savedialog.hpp"

namespace sv {
namespace ui {
namespace tabs {

DeviceTab::DeviceTab(Session &session,
		shared_ptr<sv::devices::BaseDevice> device, QMainWindow *parent) :
	BaseTab(session, parent),
	device_(device),
	action_aquire_(new QAction(this)),
	action_open_(new QAction(this)),
	action_save_as_(new QAction(this)),
	action_add_control_view_(new QAction(this)),
	action_add_panel_view_(new QAction(this)),
	action_add_plot_view_(new QAction(this)),
	action_add_math_channel_(new QAction(this)),
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
	action_aquire_->setText(tr("Stop"));
	action_aquire_->setIconText(tr("Stop"));
	action_aquire_->setIcon(QIcon(":/icons/status-green.svg"));
	action_aquire_->setCheckable(true);
	action_aquire_->setChecked(true);
	connect(action_aquire_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_aquire_triggered()));

	QToolButton *aquire_button_ = new QToolButton();
	aquire_button_->setDefaultAction(action_aquire_);
	aquire_button_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	/* TODO
	action_open_->setText(tr("&Open..."));
	action_open_->setIcon(
		QIcon::fromTheme("document-open",
		QIcon(":/icons/document-open.png")));
	action_open_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
	connect(action_open_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_open_triggered()));
	*/

	action_save_as_->setText(tr("&Save As..."));
	action_save_as_->setIconText("");
	action_save_as_->setIcon(
		QIcon::fromTheme("document-save",
		QIcon(":/icons/document-save.png")));
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
		QIcon::fromTheme("mixer-front",
		QIcon(":/icons/mixer-front.png")));
	action_add_control_view_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
	connect(action_add_control_view_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_control_view_triggered()));

	action_add_panel_view_->setText(tr("Add &Panel..."));
	action_add_panel_view_->setIcon(
		QIcon::fromTheme("chronometer",
		QIcon(":/icons/chronometer.png")));
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

	action_add_math_channel_->setText(tr("Add &Math Channel..."));
	action_add_math_channel_->setIcon(
		QIcon::fromTheme("office-chart-line-percentage",
		QIcon(":/icons/office-chart-line-percentage.png")));
	action_add_math_channel_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
	connect(action_add_math_channel_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_math_channel_triggered()));

	action_about_->setText(tr("&About..."));
	action_about_->setIcon(
		QIcon::fromTheme("help-about",
		QIcon(":/icons/help-about.png")));
	action_about_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
	connect(action_about_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_about_triggered()));

	toolbar = new QToolBar("Device Toolbar");
	toolbar->addWidget(aquire_button_);
	toolbar->addSeparator();
	//toolbar->addAction(action_open_); // TODO
	toolbar->addAction(action_save_as_);
	toolbar->addSeparator();
	toolbar->addAction(action_reset_data_);
	toolbar->addSeparator();
	toolbar->addAction(action_add_control_view_);
	toolbar->addAction(action_add_panel_view_);
	toolbar->addAction(action_add_plot_view_);
	toolbar->addSeparator();
	toolbar->addAction(action_add_math_channel_);
	toolbar->addSeparator();
	toolbar->addAction(action_about_);
	parent_->addToolBar(Qt::TopToolBarArea, toolbar);
}

void DeviceTab::on_action_aquire_triggered()
{
	if (action_aquire_->isChecked()) {
		action_aquire_->setText(tr("Stop"));
		action_aquire_->setIconText(tr("Stop"));
		action_aquire_->setIcon(QIcon(":/icons/status-green.svg"));
		device_->start_aquisition();
	}
	else {
		action_aquire_->setText(tr("Start"));
		action_aquire_->setIconText(tr("Start"));
		action_aquire_->setIcon(QIcon(":/icons/status-red.svg"));
		device_->pause_aquisition();
	}
}

void DeviceTab::on_action_open_triggered()
{
}

void DeviceTab::on_action_save_as_triggered()
{
	ui::dialogs::SaveDialog dlg(session(), device_->all_signals());
	dlg.exec();
}

void DeviceTab::on_action_add_control_view_triggered()
{
	shared_ptr<sv::devices::BaseDevice> d = nullptr;
	if (device_->type() != sv::devices::DeviceType::VirtualDevice)
		d = device_;

	ui::dialogs::AddViewDialog dlg(session(), d, 0);
	if (!dlg.exec())
		return;

	for (const auto &view : dlg.views())
		add_view(view, Qt::TopDockWidgetArea);
}

void DeviceTab::on_action_add_panel_view_triggered()
{
	shared_ptr<sv::devices::BaseDevice> d = nullptr;
	if (device_->type() != sv::devices::DeviceType::VirtualDevice)
		d = device_;

	ui::dialogs::AddViewDialog dlg(session(), d, 1);
	if (!dlg.exec())
		return;

	for (const auto &view : dlg.views())
		add_view(view, Qt::TopDockWidgetArea);
}

void DeviceTab::on_action_add_plot_view_triggered()
{
	shared_ptr<sv::devices::BaseDevice> d = nullptr;
	if (device_->type() != sv::devices::DeviceType::VirtualDevice)
		d = device_;

	ui::dialogs::AddViewDialog dlg(session(), d, 2);
	if (!dlg.exec())
		return;

	for (const auto &view : dlg.views())
		add_view(view, Qt::BottomDockWidgetArea);
}

void DeviceTab::on_action_add_math_channel_triggered()
{
	shared_ptr<sv::devices::BaseDevice> d = nullptr;
	if (device_->type() != sv::devices::DeviceType::VirtualDevice)
		d = device_;

	ui::dialogs::AddMathChannelDialog dlg(session(), d);
	if (!dlg.exec())
		return;

	auto channel = dlg.channel();
	if (channel != nullptr)
		device_->add_channel(channel, dlg.channel_group_name());
}

void DeviceTab::on_action_reset_data_triggered()
{
}

void DeviceTab::on_action_about_triggered()
{
	ui::dialogs::AboutDialog dlg(this->session().device_manager(), device_);
	dlg.exec();
}

} // namespace tabs
} // namespace ui
} // namespace sv
