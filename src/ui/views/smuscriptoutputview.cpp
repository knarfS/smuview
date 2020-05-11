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

#include <string>

#include <QAction>
#include <QBrush>
#include <QDebug>
#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QString>
#include <QTextCharFormat>
#include <QToolBar>
#include <QVBoxLayout>

#include "smuscriptoutputview.hpp"
#include "src/session.hpp"
#include "src/ui/views/baseview.hpp"

namespace sv {
namespace ui {
namespace views {

SmuScriptOutputView::SmuScriptOutputView(Session &session, QWidget *parent) :
	BaseView(session, parent),
	auto_scroll_(true),
	action_auto_scroll_(new QAction(this)),
	action_clear_output_(new QAction(this))
{
	id_ = "smuscriptoutput:" + std::to_string(BaseView::id_counter++);;

	setup_ui();
	setup_toolbar();
}

QString SmuScriptOutputView::title() const
{
	return tr("SmuScript Output");
}

void SmuScriptOutputView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	output_edit_ = new QPlainTextEdit();
	output_edit_->setReadOnly(true);
	// Same as QCodeEditor::initFont()
	auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	font.setFixedPitch(true);
	font.setPointSize(10);
	output_edit_->setFont(font);

	layout->addWidget(output_edit_);

	this->central_widget_->setLayout(layout);
}

void SmuScriptOutputView::setup_toolbar()
{
	action_auto_scroll_->setText(tr("Auto scroll"));
	action_auto_scroll_->setIcon(
		QIcon::fromTheme("go-bottom",
		QIcon(":/icons/go-bottom.png")));
	action_auto_scroll_->setCheckable(true);
	action_auto_scroll_->setChecked(auto_scroll_);
	connect(action_auto_scroll_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_auto_scroll_triggered()));

	action_clear_output_->setText(tr("Clear output"));
	action_clear_output_->setIcon(
		QIcon::fromTheme("edit-delete",
		QIcon(":/icons/edit-delete.png")));
	connect(action_clear_output_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_clear_output_triggered()));

	toolbar_ = new QToolBar("SmuScript Output Toolbar");
	toolbar_->addAction(action_auto_scroll_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_clear_output_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void SmuScriptOutputView::scroll_to_bottom()
{
	output_edit_->verticalScrollBar()->setValue(
		output_edit_->verticalScrollBar()->maximum());
}

void SmuScriptOutputView::append_out_text(const std::string &text)
{
	output_edit_->appendPlainText(QString::fromStdString(text));
	if (auto_scroll_)
		scroll_to_bottom();
}

void SmuScriptOutputView::append_err_text(const std::string &text)
{
	QTextCharFormat tcf = output_edit_->currentCharFormat();
	QBrush old_brush = tcf.foreground();
	tcf.setForeground(QBrush(Qt::red));
	output_edit_->setCurrentCharFormat(tcf);
	output_edit_->appendPlainText(QString::fromStdString(text));
	tcf.setForeground(old_brush);
	output_edit_->setCurrentCharFormat(tcf);
	if (auto_scroll_)
		scroll_to_bottom();
}

void SmuScriptOutputView::on_action_auto_scroll_triggered()
{
	auto_scroll_ = !auto_scroll_;
	action_auto_scroll_->setChecked(auto_scroll_);
}

void SmuScriptOutputView::on_action_clear_output_triggered()
{
	output_edit_->clear();
}

} // namespace views
} // namespace ui
} // namespace sv
