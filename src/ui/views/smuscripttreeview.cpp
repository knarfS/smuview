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

#include <QAbstractItemView>
#include <QAction>
#include <QDebug>
#include <QFileSystemModel>
#include <QMessageBox>
#include <QModelIndex>
#include <QString>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include "smuscripttreeview.hpp"
#include "src/mainwindow.hpp"
#include "src/session.hpp"
#include "src/python/smuscriptrunner.hpp"
#include "src/ui/views/baseview.hpp"

using std::string;

namespace sv {
namespace ui {
namespace views {

SmuScriptTreeView::SmuScriptTreeView(Session &session, QWidget *parent) :
	BaseView(session, parent),
	action_new_script_(new QAction(this)),
	action_open_script_(new QAction(this)),
	action_run_script_(new QAction(this))
{
	// TODO: Set path to example files dir (how to do this in an AppImage?)
	//       or save last directory in Session
	script_dir_ = QDir::homePath();

	setup_ui();
	setup_toolbar();
	connect_signals();
}

QString SmuScriptTreeView::title() const
{
	return tr("SmuScript");
}

void SmuScriptTreeView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	file_system_model_ = new QFileSystemModel();
	file_system_model_->setRootPath("");
	file_system_tree_ = new QTreeView();
	file_system_tree_->setModel(file_system_model_);
	file_system_tree_->setAnimated(false);
	file_system_tree_->setIndentation(20);
	file_system_tree_->setSortingEnabled(true);
	file_system_tree_->sortByColumn(0, Qt::SortOrder::AscendingOrder);
	layout->addWidget(file_system_tree_);

	layout->setContentsMargins(2, 2, 2, 2);

	this->central_widget_->setLayout(layout);

	file_system_tree_->setColumnWidth(0, file_system_tree_->width());

	// NOTE: QFileSystemModel::index() doesn't return the correct row the first
	//       time or when call a second time directly after the first.
	//       Therefore it is called via a timer the second time.
	QModelIndex script_path_index = file_system_model_->index(script_dir_);
	file_system_tree_->expand(script_path_index);
	file_system_tree_->setCurrentIndex(script_path_index);
	QTimer::singleShot(100, this, &SmuScriptTreeView::scroll_to_script_dir);
}

void SmuScriptTreeView::setup_toolbar()
{
	action_new_script_->setText(tr("New script"));
	action_new_script_->setIconText(tr("New script"));
	action_new_script_->setIcon(
		QIcon::fromTheme("document-new",
		QIcon(":/icons/document-new.png")));
	connect(action_new_script_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_new_script_triggered()));

	action_open_script_->setText(tr("Open script"));
	action_open_script_->setIconText(tr("Open script"));
	action_open_script_->setIcon(
		QIcon::fromTheme("document-open",
		QIcon(":/icons/document-open.png")));
	connect(action_open_script_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_open_script_triggered()));

	action_run_script_->setText(tr("Run script"));
	action_run_script_->setIconText(tr("Run script"));
	action_run_script_->setIcon(
		QIcon::fromTheme("media-playback-start",
		QIcon(":/icons/media-playback-start.png")));
	action_run_script_->setCheckable(true);
	connect(action_run_script_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_run_script_triggered()));
	if (session_.smu_script_runner()->is_running())
		action_run_script_->setChecked(true);
	else
		action_run_script_->setChecked(false);

	toolbar_ = new QToolBar("SmuScript Toolbar");
	toolbar_->addAction(action_new_script_);
	toolbar_->addAction(action_open_script_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_run_script_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void SmuScriptTreeView::connect_signals()
{
	connect(session_.smu_script_runner().get(), &python::SmuScriptRunner::script_started,
		this, &SmuScriptTreeView::on_script_started);
	connect(session_.smu_script_runner().get(), &python::SmuScriptRunner::script_finished,
		this, &SmuScriptTreeView::on_script_finished);
}

void SmuScriptTreeView::scroll_to_script_dir()
{
	QModelIndex script_path_index = file_system_model_->index(script_dir_);
	file_system_tree_->scrollTo(script_path_index, QAbstractItemView::PositionAtTop);
}

void SmuScriptTreeView::on_action_new_script_triggered()
{
	session().main_window()->add_smuscript_tab("");
}

void SmuScriptTreeView::on_action_open_script_triggered()
{
	QModelIndex index = file_system_tree_->selectionModel()->currentIndex();
	if (!index.isValid())
		return;

	session().main_window()->add_smuscript_tab(
		file_system_model_->filePath(index).toStdString());
}

void SmuScriptTreeView::on_action_run_script_triggered()
{
	if (action_run_script_->isChecked()) {
		QModelIndex index = file_system_tree_->selectionModel()->currentIndex();
		if (index.isValid())
			session_.smu_script_runner()->run(
				file_system_model_->filePath(index).toStdString());
	}
	else
		session_.smu_script_runner()->stop();
}

void SmuScriptTreeView::on_script_started()
{
	action_run_script_->setText(tr("Stop"));
	action_run_script_->setIconText(tr("Stop"));
	action_run_script_->setIcon(
		QIcon::fromTheme("media-playback-stop",
		QIcon(":/icons/media-playback-stop.png")));
	action_run_script_->setChecked(true);
}

void SmuScriptTreeView::on_script_finished()
{
	action_run_script_->setText(tr("Run"));
	action_run_script_->setIconText(tr("Run"));
	action_run_script_->setIcon(
		QIcon::fromTheme("media-playback-start",
		QIcon(":/icons/media-playback-start.png")));
	action_run_script_->setChecked(false);
}

} // namespace views
} // namespace ui
} // namespace sv

