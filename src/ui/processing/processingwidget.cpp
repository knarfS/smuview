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

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>

#include "processingwidget.hpp"
#include "src/session.hpp"
#include "src/processing/processor.hpp"
#include "src/ui/processing/processthreadwidget.hpp"

using std::make_shared;

namespace sv {
namespace ui {
namespace processing {

ProcessingWidget::ProcessingWidget(shared_ptr<Session> session,
		QWidget *parent) :
	QMainWindow(parent),
	session_(session),
	thread_count_(0),
	action_start_process_(new QAction(this)),
	action_pause_process_(new QAction(this)),
	action_stop_process_(new QAction(this)),
	action_add_thread_(new QAction(this)),
	action_save_process_(new QAction(this))
{
	processor_ = make_shared<sv::processing::Processor>();

	setup_ui();
	setup_toolbar();
}

void ProcessingWidget::setup_ui()
{
	//process_tab_widget_ = new QTabWidget();
	thread_toolbox_ = new QToolBox();

	thread_count_++;
	ui::processing::ProcessThreadWidget *main_thread =
		new ui::processing::ProcessThreadWidget(session_, "main", processor_);
	//process_tab_widget_->addTab(main_thread, "main");
	thread_toolbox_->addItem(main_thread, "main");

	//this->setCentralWidget(process_tab_widget_);
	this->setCentralWidget(thread_toolbox_);
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
	action_pause_process_->setDisabled(true);
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

	connect(processor_.get(), SIGNAL(processor_started()),
		this, SLOT(on_processor_started()));
	connect(processor_.get(), SIGNAL(processor_finished()),
		this, SLOT(on_processor_finished()));
}

void ProcessingWidget::on_action_start_process_triggered()
{
	if (processor_->is_running()) {
		action_start_process_->setChecked(true);
		return;
	}

	processor_->start([&](QString message) {
		processing_error(tr("Processing failed"), message);
	});
}

void ProcessingWidget::on_action_pause_process_triggered()
{
	action_start_process_->setChecked(false);
	action_stop_process_->setChecked(false);

	processor_->pause();

	action_pause_process_->setChecked(true);
}

void ProcessingWidget::on_action_stop_process_triggered()
{
	if (!processor_->is_running()) {
		action_stop_process_->setChecked(true);
		return;
	}

	processor_->stop();
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
			new ui::processing::ProcessThreadWidget(session_, name, processor_);
		//process_tab_widget_->addTab(sub_thread, name);
		thread_toolbox_->addItem(sub_thread, name);
	}
}

void ProcessingWidget::on_action_save_process_triggered()
{
}

void ProcessingWidget::on_processor_started()
{
	action_pause_process_->setChecked(false);
	action_stop_process_->setChecked(false);

	action_start_process_->setChecked(true);
}

void ProcessingWidget::on_processor_finished()
{
	action_start_process_->setChecked(false);
	action_pause_process_->setChecked(false);
	action_stop_process_->setChecked(true);
}

void ProcessingWidget::processing_error(
	const QString text, const QString info_text)
{
	QMetaObject::invokeMethod(this, "show_processing_error",
		Qt::QueuedConnection, Q_ARG(QString, text),
		Q_ARG(QString, info_text));
}

void ProcessingWidget::show_processing_error(
	const QString text, const QString info_text)
{
	QMessageBox msg(this);
	msg.setText(text);
	msg.setInformativeText(info_text);
	msg.setStandardButtons(QMessageBox::Ok);
	msg.setIcon(QMessageBox::Warning);
	msg.exec();
}

} // namespace processing
} // namespace ui
} // namespace sv
