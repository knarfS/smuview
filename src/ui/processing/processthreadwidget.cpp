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
#include <QFormLayout>
#include <QMenu>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "processthreadwidget.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/processing/processor.hpp"
#include "src/processing/setvalueblock.hpp"
#include "src/processing/stepblock.hpp"
#include "src/processing/waitblock.hpp"
#include "src/ui/processing/dialogs/setvalueblockdialog.hpp"
#include "src/ui/processing/dialogs/stepblockdialog.hpp"
#include "src/ui/processing/dialogs/waitblockdialog.hpp"
#include "src/ui/processing/items/sequencesinitem.hpp"
#include "src/ui/processing/items/stepitem.hpp"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::unordered_set;

namespace sv {
namespace ui {
namespace processing {

ProcessThreadWidget::ProcessThreadWidget(shared_ptr<Session> session,
		QString name, shared_ptr<sv::processing::Processor> processor,
		QWidget *parent) :
	QMainWindow(parent),
	session_(session),
	name_(name),
	processor_(processor),
	action_add_set_value_block_(new QAction(this)),
	action_add_get_value_block_(new QAction(this)),
	action_add_step_block_(new QAction(this)),
	action_add_sequence_block_(new QAction(this)),
	action_add_wait_block_(new QAction(this)),
	action_add_user_input_block_(new QAction(this)),
	action_add_create_signal_block_(new QAction(this)),
	action_remove_block_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
}

void ProcessThreadWidget::setup_ui()
{
	QVBoxLayout *main_layout = new QVBoxLayout();

	QFormLayout *form_layout = new QFormLayout();

	name_edit_ = new QLineEdit();
	name_edit_->setText(name_);
	form_layout->addRow(tr("Name"), name_edit_);

	run_at_startup_checkbox_ = new QCheckBox();
	form_layout->addRow(tr("Run at startup"), run_at_startup_checkbox_);

	main_layout->addLayout(form_layout);

	process_block_list_ = new QListWidget();
	process_block_list_->setIconSize(QSize(32, 32));
	main_layout->addWidget(process_block_list_);

	QWidget *main_widget = new QWidget();
	main_widget->setLayout(main_layout);
	this->setCentralWidget(main_widget);
}

void ProcessThreadWidget::setup_toolbar()
{
	QMenu *add_menu = new QMenu();

	action_add_set_value_block_->setText(tr("Add Set Value block"));
	connect(action_add_set_value_block_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_set_value_block_triggered()));
	add_menu->addAction(action_add_set_value_block_);

	action_add_get_value_block_->setText(tr("Add Get Value block"));
	connect(action_add_get_value_block_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_get_value_block_triggered()));
	add_menu->addAction(action_add_get_value_block_);

	action_add_step_block_->setText(tr("Add Step block"));
	connect(action_add_step_block_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_step_block_triggered()));
	add_menu->addAction(action_add_step_block_);

	action_add_sequence_block_->setText(tr("Add Sequence block"));
	connect(action_add_sequence_block_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_sequence_block_triggered()));
	add_menu->addAction(action_add_sequence_block_);

	action_add_wait_block_->setText(tr("Add Wait block"));
	connect(action_add_wait_block_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_wait_block_triggered()));
	add_menu->addAction(action_add_wait_block_);

	action_add_user_input_block_->setText(tr("Add User Input block"));
	connect(action_add_user_input_block_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_user_input_block_triggered()));
	add_menu->addAction(action_add_user_input_block_);

	action_add_create_signal_block_->setText(tr("Add Create Signal block"));
	connect(action_add_create_signal_block_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_create_signal_block_triggered()));
	add_menu->addAction(action_add_create_signal_block_);

	add_button_ = new QToolButton();
	add_button_->setText(tr("Add block"));
	add_button_->setIcon(
		QIcon::fromTheme("list-add",
		QIcon(":/icons/list-add.png")));
	add_button_->setMenu(add_menu);
	add_button_->setPopupMode(QToolButton::MenuButtonPopup);

	action_remove_block_->setText(tr("Remove block"));
	action_remove_block_->setIcon(
		QIcon::fromTheme("list-remove",
		QIcon(":/icons/list-remove.png")));
	connect(action_remove_block_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_remove_block_triggered()));

	toolbar_ = new QToolBar("Thread Toolbar");
	toolbar_->addWidget(add_button_);
	toolbar_->addAction(action_remove_block_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void ProcessThreadWidget::on_action_add_set_value_block_triggered()
{
	dialogs::SetValueBlockDialog dlg(session_);
	if (!dlg.exec())
		return;

	shared_ptr<sv::processing::SetValueBlock> block =
		make_shared<sv::processing::SetValueBlock>(processor_);
	block->set_property(dlg.property());
	block->set_value(dlg.value());
	processor_->add_block_to_process(block);

	ui::processing::items::StepItem *item =
		new ui::processing::items::StepItem();
	item->set_block(block);
	process_block_list_->addItem(item);
}

void ProcessThreadWidget::on_action_add_get_value_block_triggered()
{
}

void ProcessThreadWidget::on_action_add_step_block_triggered()
{
	dialogs::StepBlockDialog dlg(session_);
	if (!dlg.exec())
		return;

	shared_ptr<sv::processing::StepBlock> block =
		make_shared<sv::processing::StepBlock>(processor_);
	block->set_property(dlg.property());
	block->set_start_value(dlg.start_value());
	block->set_end_value(dlg.end_value());
	block->set_step_size(dlg.step_size());
	block->set_delay_ms(dlg.delay_ms());
	processor_->add_block_to_process(block);

	ui::processing::items::StepItem *item =
		new ui::processing::items::StepItem();
	item->set_block(block);
	process_block_list_->addItem(item);
}

void ProcessThreadWidget::on_action_add_sequence_block_triggered()
{
}

void ProcessThreadWidget::on_action_add_wait_block_triggered()
{
	dialogs::WaitBlockDialog dlg(session_);
	if (!dlg.exec())
		return;

	shared_ptr<sv::processing::WaitBlock> block =
		make_shared<sv::processing::WaitBlock>(processor_);
	block->set_wait_ms(dlg.wait_ms());
	processor_->add_block_to_process(block);

	ui::processing::items::StepItem *item =
		new ui::processing::items::StepItem();
	item->set_block(block);
	process_block_list_->addItem(item);
}

void ProcessThreadWidget::on_action_add_user_input_block_triggered()
{
}

void ProcessThreadWidget::on_action_add_create_signal_block_triggered()
{
}

void ProcessThreadWidget::on_action_remove_block_triggered()
{
}

} // namespace processing
} // namespace ui
} // namespace sv
