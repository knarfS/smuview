/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <QInputDialog>
#include <QString>

#include "processingwidget.hpp"
#include "src/ui/processing/processthreadwidget.hpp"

namespace sv {
namespace ui {
namespace processing {

ProcessingWidget::ProcessingWidget(
		QWidget *parent) :
	QMainWindow(parent),
	thread_count_(0),
	action_start_process_(new QAction(this)),
	action_pause_process_(new QAction(this)),
	action_stop_process_(new QAction(this)),
	action_add_thread_(new QAction(this)),
	action_save_process_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
}

void ProcessingWidget::setup_ui()
{
	process_tab_widget_ = new QTabWidget();

	// Add main process tab
	thread_count_++;
	ui::processing::ProcessThreadWidget *main_thread =
		new ui::processing::ProcessThreadWidget("main");
	process_tab_widget_->addTab(main_thread, "main");

	this->setCentralWidget(process_tab_widget_);
}

void ProcessingWidget::setup_toolbar()
{
	action_start_process_->setText(tr("Start process"));
	action_start_process_->setIcon(
		QIcon::fromTheme("media-playback-start",
		QIcon(":/icons/media-playback-start.png")));
	action_start_process_->setCheckable(true);
	action_start_process_->setChecked(false);
	connect(action_start_process_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_start_process_triggered()));

	action_pause_process_->setText(tr("Pause process"));
	action_pause_process_->setIcon(
		QIcon::fromTheme("media-playback-pause",
		QIcon(":/icons/media-playback-pause.png")));
	action_pause_process_->setCheckable(true);
	action_pause_process_->setChecked(false);
	connect(action_pause_process_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_pause_process_triggered()));

	action_stop_process_->setText(tr("Stop process"));
	action_stop_process_->setIcon(
		QIcon::fromTheme("media-playback-stop",
		QIcon(":/icons/media-playback-stop.png")));
	action_stop_process_->setCheckable(true);
	action_stop_process_->setChecked(true);
	connect(action_stop_process_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_stop_process_triggered()));

	action_add_thread_->setText(tr("Add thread"));
	action_add_thread_->setIcon(
		QIcon::fromTheme("list-add",
		QIcon(":/icons/list-add.png")));
	connect(action_add_thread_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_thread_triggered()));

	action_save_process_->setText(tr("Save process"));
	action_save_process_->setIcon(
		QIcon::fromTheme("document-save",
		QIcon(":/icons/document-save.png")));
	connect(action_save_process_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_start_process_triggered()));

	toolbar_ = new QToolBar("Processing Toolbar");
	toolbar_->addAction(action_start_process_);
	toolbar_->addAction(action_pause_process_);
	toolbar_->addAction(action_stop_process_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_add_thread_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_save_process_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void ProcessingWidget::on_action_start_process_triggered()
{
	action_pause_process_->setChecked(false);
	action_stop_process_->setChecked(false);

	action_start_process_->setChecked(true);
}

void ProcessingWidget::on_action_pause_process_triggered()
{
	action_start_process_->setChecked(false);
	action_stop_process_->setChecked(false);

	action_pause_process_->setChecked(true);
}

void ProcessingWidget::on_action_stop_process_triggered()
{
	action_start_process_->setChecked(false);
	action_pause_process_->setChecked(false);

	action_stop_process_->setChecked(true);
}

void ProcessingWidget::on_action_add_thread_triggered()
{
	bool ok;
	QString name = QString(tr("subthread %1")).arg(thread_count_);

	name = QInputDialog::getText(this,
		tr("New process thread"), tr("Thread name:"),
		QLineEdit::Normal, name, &ok);

	if (ok && !name.isEmpty()) {
		thread_count_++;
		ui::processing::ProcessThreadWidget *sub_thread =
			new ui::processing::ProcessThreadWidget(name);
		process_tab_widget_->addTab(sub_thread, name);
	}
}

void ProcessingWidget::on_action_save_process_triggered()
{
}

} // namespace processing
} // namespace ui
} // namespace sv
