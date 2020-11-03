/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <algorithm>
#include <string>

#include <QDebug>
#include <QDockWidget>
#include <QString>

#include "smuscripttab.hpp"
#include "src/mainwindow.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/python/smuscriptrunner.hpp"
#include "src/ui/tabs/basetab.hpp"
#include "src/ui/tabs/tabdockwidget.hpp"
#include "src/ui/views/smuscriptoutputview.hpp"
#include "src/ui/views/smuscriptview.hpp"
#include "src/ui/views/viewhelper.hpp"

using std::string;

namespace sv {
namespace ui {
namespace tabs {

SmuScriptTab::SmuScriptTab(Session &session,
		const string &script_file_name, QWidget *parent) :
	BaseTab(session, parent),
	script_file_name_(script_file_name)
{
	// The id is the same for all SmuScriptTabs, so they all look the same when
	// restored from the settings, independed of the filename.
	id_ = "smuscripttab:";

	setup_ui();
	connect_signals();

	QSettings settings;
	if (SettingsManager::restore_settings() &&
			settings.childGroups().contains("SmuScriptTab")) {
		SmuScriptTab::restore_settings();
	}
}


QString SmuScriptTab::title()
{
	return smu_script_view_->title();
}

bool SmuScriptTab::request_close()
{
	return smu_script_view_->ask_to_save(tr("Close SmuScript tab"));
}

void SmuScriptTab::setup_ui()
{
	smu_script_view_ = new views::SmuScriptView(session_);
	smu_script_view_->load_file(script_file_name_);
	add_view(smu_script_view_, Qt::TopDockWidgetArea,
		QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

	smu_script_output_view_ = new views::SmuScriptOutputView(session_);
	add_view(smu_script_output_view_, Qt::BottomDockWidgetArea,
		QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
}

void SmuScriptTab::connect_signals()
{
	connect(smu_script_view_, &views::SmuScriptView::file_name_changed,
		this, &SmuScriptTab::on_file_name_changed);
	connect(smu_script_view_, &views::SmuScriptView::file_save_state_changed,
		this, &SmuScriptTab::on_file_save_state_changed);

	// This is for redirecting the python output
	connect(smu_script_view_, &views::SmuScriptView::script_started,
		this, &SmuScriptTab::on_script_started);
	connect(smu_script_view_, &views::SmuScriptView::script_finished,
		this, &SmuScriptTab::on_script_finished);
}

void SmuScriptTab::run_script()
{
	smu_script_view_->run_script();
}

void SmuScriptTab::stop_script()
{
	smu_script_view_->stop_script();
}

void SmuScriptTab::restore_settings()
{
	QSettings settings;

	settings.beginGroup("SmuScriptTab");

	// Both views are fixed and will not be restored by their uuid, to give all
	// SmuScriptTabs the same setings.

	settings.beginGroup("view0");
	smu_script_view_->restore_settings(settings);
	settings.endGroup();

	settings.beginGroup("view1");
	smu_script_output_view_->restore_settings(settings);
	settings.endGroup();

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

void SmuScriptTab::save_settings() const
{
	QSettings settings;

	settings.beginGroup("SmuScriptTab");
	settings.remove("");

	// Both views are fixed and will not be restored by their uuid, to give all
	// SmuScriptTabs the same setings.

	settings.beginGroup("view0");
	smu_script_view_->save_settings(settings);
	settings.endGroup();

	settings.beginGroup("view1");
	smu_script_output_view_->save_settings(settings);
	settings.endGroup();

	// Save state and geometry for all view widgets.
	// NOTE: geometry must be saved. See restore_settings().
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());

	settings.endGroup();
}

void SmuScriptTab::on_file_name_changed(const QString &file_name)
{
	(void)file_name;
	view_docks_map_[smu_script_view_]->setWindowTitle(smu_script_view_->title());
	session_.main_window()->change_tab_title(id_, smu_script_view_->title());
}

void SmuScriptTab::on_file_save_state_changed(bool is_unsaved)
{
	if (is_unsaved)
		session_.main_window()->change_tab_icon(id_,
			QIcon::fromTheme("document-save", QIcon(":/icons/document-save.png")));
	else
		session_.main_window()->change_tab_icon(id_, QIcon());
}

void SmuScriptTab::on_script_started()
{
	// Redirect python output to SmuScriptOutputView
	connect(session_.smu_script_runner().get(), &python::SmuScriptRunner::send_py_stdout,
		smu_script_output_view_, &views::SmuScriptOutputView::append_out_text);
	connect(session_.smu_script_runner().get(), &python::SmuScriptRunner::send_py_stderr,
		smu_script_output_view_, &views::SmuScriptOutputView::append_err_text);
}

void SmuScriptTab::on_script_finished()
{
	disconnect(session_.smu_script_runner().get(), &python::SmuScriptRunner::send_py_stdout,
		smu_script_output_view_, &views::SmuScriptOutputView::append_out_text);
	disconnect(session_.smu_script_runner().get(), &python::SmuScriptRunner::send_py_stderr,
		smu_script_output_view_, &views::SmuScriptOutputView::append_err_text);
}

} // namespace tabs
} // namespace ui
} // namespace sv
