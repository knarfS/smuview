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
#include <QFont>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "smuscripttab.hpp"
#include "src/session.hpp"
#include "src/python/smuscriptrunner.hpp"
#include "src/ui/tabs/basetab.hpp"
#include "src/ui/widgets/scripteditor/smuscripteditor.hpp"

using std::make_shared;
using std::shared_ptr;

namespace sv {
namespace ui {
namespace tabs {

SmuScriptTab::SmuScriptTab(Session &session,
		string script_file_name, QMainWindow *parent) :
	BaseTab(session, parent),
	script_file_name_(script_file_name),
	action_open_(new QAction(this)),
	action_save_(new QAction(this)),
	action_run_(new QAction(this))
{
	smu_script_runner_ = make_shared<python::SmuScriptRunner>(session_);
	connect(smu_script_runner_.get(), SIGNAL(script_error(QString)),
		this, SLOT(on_script_error(QString)));

	setup_ui();
	setup_toolbar();
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
	this->show();
	this->setLayout(layout);
}

void SmuScriptTab::setup_toolbar()
{
	action_open_->setText(tr("&Open..."));
	action_open_->setIcon(
		QIcon::fromTheme("document-open",
		QIcon(":/icons/document-open.png")));
	connect(action_open_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_open_triggered()));

	action_save_->setText(tr("&Save As..."));
	action_save_->setIconText("");
	action_save_->setIcon(
		QIcon::fromTheme("document-save",
		QIcon(":/icons/document-save.png")));
	connect(action_save_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_save_triggered()));

	action_run_->setText(tr("Start"));
	action_run_->setIcon(
		QIcon::fromTheme("media-playback-start",
		QIcon(":/icons/media-playback-start.png")));
	action_run_->setCheckable(true);
	action_run_->setChecked(false);
	connect(action_run_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_run_triggered()));

	toolbar_ = new QToolBar("SmuScript Toolbar");
	toolbar_->addAction(action_open_);
	toolbar_->addAction(action_save_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_run_);
	parent_->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void SmuScriptTab::on_action_open_triggered()
{
    QFile file(
		"/home/frank/Projekte/elektronik/sigrok/smuview/smuscript/test2.py");
		//"/home/frank/Projekte/elektronik/sigrok/smuview/smuscript/example1.py");
    if (file.open(QFile::ReadOnly | QFile::Text))
        editor_->setPlainText(file.readAll());
}

void SmuScriptTab::on_action_save_triggered()
{
	QFile file(QString::fromStdString(script_file_name_));
    if (file.open(QFile::ReadWrite | QFile::Text)) { // QIODevice::ReadWrite
		 QTextStream stream(&file);
        stream << editor_->toPlainText() << endl;
    }
}

void SmuScriptTab::on_action_run_triggered()
{
	if (action_run_->isChecked()) {
		action_run_->setText(tr("Stop"));
		action_run_->setIconText(tr("Stop"));
		action_run_->setIcon(
			QIcon::fromTheme("media-playback-stop",
			QIcon(":/icons/media-playback-stop.png")));

		smu_script_runner_->run(script_file_name_);
	}
	else {
		action_run_->setText(tr("Start"));
		action_run_->setIconText(tr("Start"));
		action_run_->setIcon(
			QIcon::fromTheme("media-playback-start",
			QIcon(":/icons/media-playback-start.png")));
	}
}

void SmuScriptTab::on_script_error(QString msg)
{
	QMessageBox::critical(this, tr("SmuScript Error"), msg);
}

} // namespace tabs
} // namespace ui
} // namespace sv
