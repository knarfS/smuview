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

#include <memory>
#include <string>

#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "smuscripttab.hpp"
#include "src/mainwindow.hpp"
#include "src/session.hpp"
#include "src/python/smuscriptrunner.hpp"
#include "src/ui/tabs/basetab.hpp"
#include "src/ui/widgets/scripteditor/smuscripteditor.hpp"

namespace sv {
namespace ui {
namespace tabs {

SmuScriptTab::SmuScriptTab(Session &session,
		string script_file_name, QWidget *parent) :
	BaseTab(session, parent),
	script_file_name_(script_file_name),
	action_open_(new QAction(this)),
	action_save_(new QAction(this)),
	action_save_as_(new QAction(this)),
	action_run_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
	connect_signals();
}


bool SmuScriptTab::request_close()
{
	QMessageBox::StandardButton reply = QMessageBox::question(this,
		tr("Close SmuScript tab"),
		tr("The document \"%1\" has unsaved changes. Would you like to save them?").
			arg(QString::fromStdString(script_file_name_)),
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

	if (reply == QMessageBox::Yes) {
		on_action_save_triggered();
		return true;
	}
	else if (reply == QMessageBox::No)
		return true;
	return false;
}

void SmuScriptTab::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();
	editor_ = new widgets::scripteditor::SmuScriptEditor();
	if (!script_file_name_.empty()) {
		QFile script_file(script_file_name_.c_str());
		if (script_file.open(QFile::ReadOnly | QFile::Text))
			editor_->setPlainText(script_file.readAll());
	}
	layout->addWidget(editor_);

	// Show the central widget of the tab (hidden by BaseTab)
	this->centralWidget()->show();
	this->centralWidget()->setLayout(layout);
}

void SmuScriptTab::setup_toolbar()
{
	action_open_->setText(tr("&Open"));
	action_open_->setIcon(
		QIcon::fromTheme("document-open",
		QIcon(":/icons/document-open.png")));
	action_open_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
	connect(action_open_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_open_triggered()));

	action_save_->setText(tr("&Save"));
	action_save_->setIconText("");
	action_save_->setIcon(
		QIcon::fromTheme("document-save",
		QIcon(":/icons/document-save.png")));
	action_save_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	connect(action_save_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_save_triggered()));

	action_save_as_->setText(tr("Save &As"));
	action_save_as_->setIconText("");
	action_save_as_->setIcon(
		QIcon::fromTheme("document-save-as",
		QIcon(":/icons/document-save-as.png")));
	action_save_as_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
	connect(action_save_as_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_save_as_triggered()));

	action_run_->setText(tr("Start"));
	action_run_->setIcon(
		QIcon::fromTheme("media-playback-start",
		QIcon(":/icons/media-playback-start.png")));
	action_run_->setCheckable(true);
	action_run_->setChecked(false);
	connect(action_run_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_run_triggered()));
	if (session_.smu_script_runner()->is_running())
		action_run_->setDisabled(true);

	toolbar_ = new QToolBar("SmuScript Toolbar");
	toolbar_->addAction(action_open_);
	toolbar_->addAction(action_save_);
	toolbar_->addAction(action_save_as_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_run_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void SmuScriptTab::connect_signals()
{
	connect(editor_, &widgets::scripteditor::SmuScriptEditor::textChanged,
		this, &SmuScriptTab::on_text_changed);

	connect(session_.smu_script_runner().get(), &python::SmuScriptRunner::script_started,
		this, &SmuScriptTab::on_script_started);
	connect(session_.smu_script_runner().get(), &python::SmuScriptRunner::script_finished,
		this, &SmuScriptTab::on_script_finished);
}

void SmuScriptTab::on_action_open_triggered()
{
	QString file_name = QFileDialog::getOpenFileName(this,
		tr("Open SmuScript-File"), QDir::homePath(), tr("Python Files (*.py)"));

	if (file_name.length() > 0) {
		script_file_name_ = file_name.toStdString();
		QFile file(file_name);
		if (file.open(QFile::ReadOnly | QFile::Text)) {
			editor_->setPlainText(file.readAll());
			file.close();
		}
	}
}

void SmuScriptTab::on_action_save_triggered()
{
	QFile file(QString::fromStdString(script_file_name_));
	if (file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
		QTextStream stream(&file);
		stream << editor_->toPlainText() << flush;
		file.close();

		// TODO: Centralize id generation (see MainWindow::add_smusript_tab())
		string tab_id = "smuscripttab" + script_file_name_;
		session_.main_window()->change_tab_icon(tab_id, QIcon());
	}
}

void SmuScriptTab::on_action_save_as_triggered()
{
	QString file_name = QFileDialog::getSaveFileName(this,
		tr("Save SmuScript-File"), QDir::homePath(), tr("Python Files (*.py)"));

	if (file_name.length() > 0) {
		script_file_name_ = file_name.toStdString();
		QFile file(file_name);
		if (file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
			QTextStream stream(&file);
			stream << editor_->toPlainText() << flush;
			file.close();

			// TODO: Centralize id generation (see MainWindow::add_smusript_tab())
			string tab_id = "smuscripttab" + script_file_name_;
			session_.main_window()->change_tab_icon(tab_id, QIcon());
		}
	}
}

void SmuScriptTab::on_text_changed()
{
	// TODO: Centralize id generation (see MainWindow::add_smusript_tab())
	string tab_id = "smuscripttab" + script_file_name_;
	session_.main_window()->change_tab_icon(tab_id,
		QIcon::fromTheme("document-save",
			QIcon(":/icons/document-save.png")));
}

void SmuScriptTab::on_action_run_triggered()
{
	if (action_run_->isChecked()) {
		action_run_->setText(tr("Running"));
		action_run_->setIconText(tr("Running"));
		/*
		 * TODO: Script cannot be stopped (yet?).
		action_run_->setText(tr("Stop"));
		action_run_->setIconText(tr("Stop"));
		action_run_->setIcon(
			QIcon::fromTheme("media-playback-stop",
			QIcon(":/icons/media-playback-stop.png")));
		*/

		session_.smu_script_runner()->run(script_file_name_);
	}
	else {
		action_run_->setText(tr("Start"));
		action_run_->setIconText(tr("Start"));
		/*
		 * TODO: Script cannot be stopped (yet?).
		action_run_->setIcon(
			QIcon::fromTheme("media-playback-start",
			QIcon(":/icons/media-playback-start.png")));

		session_.smu_script_runner()->stop();
		*/
	}
}

void SmuScriptTab::on_script_started()
{
	action_run_->setDisabled(true);
}

void SmuScriptTab::on_script_finished()
{
	action_run_->setDisabled(false);
	action_run_->setChecked(false);
}

} // namespace tabs
} // namespace ui
} // namespace sv
