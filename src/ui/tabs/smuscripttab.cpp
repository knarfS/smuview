/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <string>

#include <QDebug>
#include <QDockWidget>
#include <QString>

#include "smuscripttab.hpp"
#include "src/mainwindow.hpp"
#include "src/session.hpp"
#include "src/python/smuscriptrunner.hpp"
#include "src/ui/tabs/basetab.hpp"
#include "src/ui/views/smuscriptoutputview.hpp"
#include "src/ui/views/smuscriptview.hpp"

using std::string;

namespace sv {
namespace ui {
namespace tabs {

unsigned int SmuScriptTab::smuscript_tab_counter_ = 0;

SmuScriptTab::SmuScriptTab(Session &session,
		string script_file_name, QWidget *parent) :
	BaseTab(session, parent),
	script_file_name_(script_file_name)
{
	// Every script tab gets its own unique id
	tab_id_ = "smuscripttab:" +
		std::to_string(SmuScriptTab::smuscript_tab_counter_++);

	setup_ui();
	connect_signals();
}


string SmuScriptTab::tab_id()
{
	return tab_id_;
}

QString SmuScriptTab::tab_title()
{
	return smu_script_view_->title();
}

bool SmuScriptTab::request_close()
{
	return smu_script_view_->ask_to_save(tr("Close SmuScript tab"));
}

void SmuScriptTab::setup_ui()
{
	smu_script_view_ = new views::SmuScriptView(session_, script_file_name_);
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

void SmuScriptTab::on_file_name_changed(const QString &file_name)
{
	(void)file_name;
	session_.main_window()->change_tab_title(tab_id_, smu_script_view_->title());
}

void SmuScriptTab::on_file_save_state_changed(bool is_unsaved)
{
	if (is_unsaved)
		session_.main_window()->change_tab_icon(tab_id_,
			QIcon::fromTheme("document-save", QIcon(":/icons/document-save.png")));
	else
		session_.main_window()->change_tab_icon(tab_id_, QIcon());
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
