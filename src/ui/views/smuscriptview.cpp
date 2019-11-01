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

#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QString>
#include <QToolBar>
#include <QVBoxLayout>

#include "smuscriptview.hpp"
#include "src/mainwindow.hpp"
#include "src/session.hpp"
#include "src/python/smuscriptrunner.hpp"
#include "src/ui/views/baseview.hpp"

using std::make_shared;
using std::shared_ptr;
using std::string;

namespace sv {
namespace ui {
namespace views {

SmuScriptView::SmuScriptView(Session &session, QWidget *parent) :
	BaseView(session, parent),
	action_start_script_(new QAction(this))
{
	smu_script_runner_ = make_shared<python::SmuScriptRunner>(session_);
	connect(smu_script_runner_.get(), SIGNAL(script_error(QString)),
		this, SLOT(on_script_error(QString)));

	setup_ui();
	setup_toolbar();
	connect_signals();
}

QString SmuScriptView::title() const
{
	return tr("SmuScript");
}

void SmuScriptView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();
	//layout->addWidget(device_tree_);
	layout->setContentsMargins(2, 2, 2, 2);

	this->central_widget_->setLayout(layout);
}

void SmuScriptView::setup_toolbar()
{
	action_start_script_->setText(tr("Start script"));
	action_start_script_->setIcon(
		QIcon::fromTheme("media-playback-start",
		QIcon(":/icons/media-playback-start.png")));
	connect(action_start_script_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_start_script_triggered()));

	toolbar_ = new QToolBar("SmuScript Toolbar");
	toolbar_->addAction(action_start_script_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void SmuScriptView::connect_signals()
{
}

void SmuScriptView::on_action_start_script_triggered()
{
	string file_name =
		"/home/frank/Projekte/elektronik/sigrok/smuview/smuscript/test1.py";
		//"/home/frank/Projekte/elektronik/sigrok/smuview/smuscript/test2.py";
		//"/home/frank/Projekte/elektronik/sigrok/smuview/smuscript/example1.py";
	smu_script_runner_->run(file_name);
}

void SmuScriptView::on_script_error(QString msg)
{
	QMessageBox::critical(this, tr("SmuScript Error"), msg);
}

} // namespace views
} // namespace ui
} // namespace sv

