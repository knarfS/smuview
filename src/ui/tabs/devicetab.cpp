/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>

#include <QCloseEvent>
#include <QDebug>
#include <QMainWindow>
#include <QMessageBox>
#include <QToolButton>
#include <QWidget>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "devicetab.hpp"
#include "src/session.hpp"
#include "src/channels/userchannel.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/dialogs/aboutdialog.hpp"
#include "src/ui/dialogs/addmathchanneldialog.hpp"
#include "src/ui/dialogs/addviewdialog.hpp"
#include "src/ui/dialogs/savedialog.hpp"
#include "src/ui/tabs/basetab.hpp"
#include "src/ui/tabs/tabdockwidget.hpp"
#include "src/ui/views/viewhelper.hpp"

using std::shared_ptr;

namespace sv {
namespace ui {
namespace tabs {

DeviceTab::DeviceTab(Session &session,
		shared_ptr<sv::devices::BaseDevice> device, QWidget *parent) :
	BaseTab(session, parent),
	device_(device),
	action_aquire_(new QAction(this)),
	action_save_as_(new QAction(this)),
	action_add_control_view_(new QAction(this)),
	action_add_panel_view_(new QAction(this)),
	action_add_plot_view_(new QAction(this)),
	action_add_table_view_(new QAction(this)),
	action_add_math_channel_(new QAction(this)),
	action_about_(new QAction(this))
{
	id_ = "devicetab:" + device_->id();

	setup_toolbar();
}


QString DeviceTab::title()
{
	return device_->short_name();
}

bool DeviceTab::request_close()
{
	QMessageBox::StandardButton reply = QMessageBox::information(this,
		tr("Close device tab"),
		tr("Closing the device tab will leave the device connected!"),
		QMessageBox::Ok | QMessageBox::Cancel);

	return reply == QMessageBox::Ok;
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

	action_save_as_->setText(tr("&Save As..."));
	action_save_as_->setIconText("");
	action_save_as_->setIcon(
		QIcon::fromTheme("document-save",
		QIcon(":/icons/document-save.png")));
	action_save_as_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	connect(action_save_as_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_save_as_triggered()));

	action_add_control_view_->setText(tr("Add Control"));
	action_add_control_view_->setIcon(
		QIcon::fromTheme("mixer-front",
		QIcon(":/icons/mixer-front.png")));
	connect(action_add_control_view_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_control_view_triggered()));

	action_add_panel_view_->setText(tr("Add Panel"));
	action_add_panel_view_->setIcon(
		QIcon::fromTheme("chronometer",
		QIcon(":/icons/chronometer.png")));
	connect(action_add_panel_view_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_panel_view_triggered()));

	action_add_plot_view_->setText(tr("Add Plot"));
	action_add_plot_view_->setIcon(
		QIcon::fromTheme("office-chart-line",
		QIcon(":/icons/office-chart-line.png")));
	connect(action_add_plot_view_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_plot_view_triggered()));

	action_add_table_view_->setText(tr("Add data table"));
	action_add_table_view_->setIcon(
		QIcon::fromTheme("view-form-table",
		QIcon(":/icons/view-form-table.png")));
	connect(action_add_table_view_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_table_view_triggered()));

	action_add_math_channel_->setText(tr("Add Math Channel"));
	action_add_math_channel_->setIcon(
		QIcon::fromTheme("office-chart-line-percentage",
		QIcon(":/icons/office-chart-line-percentage.png")));
	connect(action_add_math_channel_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_math_channel_triggered()));

	action_about_->setText(tr("About"));
	action_about_->setIcon(
		QIcon::fromTheme("help-about",
		QIcon(":/icons/help-about.png")));
	connect(action_about_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_about_triggered()));

	toolbar_ = new QToolBar("Device Toolbar");
	// objectName is needed for QSettings
	toolbar_->setObjectName("toolbar:" + QString::fromStdString(id()));
	toolbar_->addWidget(aquire_button_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_save_as_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_add_control_view_);
	toolbar_->addAction(action_add_panel_view_);
	toolbar_->addAction(action_add_plot_view_);
	toolbar_->addAction(action_add_table_view_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_add_math_channel_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_about_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void DeviceTab::restore_settings()
{
	qWarning() << "DeviceTab::restore_settings(): " <<
		QString::fromStdString(device_->id());

	QSettings settings;

	// Restore device views
	settings.beginGroup(QString::fromStdString(device_->id()));

	QStringList view_keys = settings.childGroups();
	for (const auto &view_key : view_keys) {
		settings.beginGroup(view_key);
		qWarning() << "DeviceTab::restore_settings(): view_key = " << view_key;
		auto view = views::viewhelper::get_view_from_settings(session_, settings);
		if (view)
			add_view(view, Qt::DockWidgetArea::TopDockWidgetArea);
		settings.endGroup();
	}

	// Restore state and geometry for all view widgets.
	// NOTE: restoreGeometry() must be called _and_ the sizeHint() of the widget
	//       (view) must be set to the last size, in order to restore the
	//       correct size of the dock widget. Calling/Setting only one of them
	//       is not working!
	if (settings.contains("geometry"))
		restoreGeometry(settings.value("geometry").toByteArray());
	if (settings.contains("state"))
		restoreState(settings.value("state").toByteArray());

	settings.endGroup();
}

void DeviceTab::save_settings() const
{
	qWarning() << "DeviceTab::save_settings(): " <<
		QString::fromStdString(device_->id());

	QSettings settings;

	settings.beginGroup(QString::fromStdString(device_->id()));
	settings.remove("");  // Remove all keys in this group

	size_t i = 0;
	for (const auto &view_dock_pair : view_docks_map_) {
		qWarning() << "DeviceTab::save_settings(): group = " << QString("view%1").arg(i);
		qWarning() << "DeviceTab::save_settings(): view type = " << QString::fromStdString(view_dock_pair.first->id());
		settings.beginGroup(QString("view%1").arg(i));
		view_dock_pair.first->save_settings(settings);
		settings.endGroup();
		++i;
	}

	// Save state and geometry for all view widgets.
	// NOTE: geometry must be saved. See restore_settings().
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());

	settings.endGroup();
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

void DeviceTab::on_action_save_as_triggered()
{
	ui::dialogs::SaveDialog dlg(session(), device_);
	dlg.exec();
}

void DeviceTab::on_action_add_control_view_triggered()
{
	shared_ptr<sv::devices::BaseDevice> d = nullptr;
	if (device_->type() != sv::devices::DeviceType::UserDevice)
		d = device_;

	ui::dialogs::AddViewDialog dlg(session(), d, 0);
	if (!dlg.exec())
		return;

	for (const auto &view : dlg.views())
		add_view(view, Qt::TopDockWidgetArea);
}

void DeviceTab::on_action_add_panel_view_triggered()
{
	ui::dialogs::AddViewDialog dlg(session(), device_, 2);
	if (!dlg.exec())
		return;

	for (const auto &view : dlg.views())
		add_view(view, Qt::TopDockWidgetArea);
}

void DeviceTab::on_action_add_plot_view_triggered()
{
	ui::dialogs::AddViewDialog dlg(session(), device_, 3);
	if (!dlg.exec())
		return;

	for (const auto &view : dlg.views())
		add_view(view, Qt::BottomDockWidgetArea);
}

void DeviceTab::on_action_add_table_view_triggered()
{
	ui::dialogs::AddViewDialog dlg(session(), device_, 5);
	if (!dlg.exec())
		return;

	for (const auto &view : dlg.views())
		add_view(view, Qt::TopDockWidgetArea);
}

void DeviceTab::on_action_add_math_channel_triggered()
{
	ui::dialogs::AddMathChannelDialog dlg(session(), device_);
	if (!dlg.exec())
		return;

	auto channel = dlg.channel();
	if (channel != nullptr) {
		device_->add_math_channel(
			channel, dlg.channel_group_name().toStdString());
	}
}

void DeviceTab::on_action_about_triggered()
{
	ui::dialogs::AboutDialog dlg(this->session().device_manager(), device_);
	dlg.exec();
}

} // namespace tabs
} // namespace ui
} // namespace sv
