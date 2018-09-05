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

#ifndef UI_PROCESSING_PROCESSTHREADWIDGET_HPP
#define UI_PROCESSING_PROCESSTHREADWIDGET_HPP

#include <memory>

#include <QAction>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QString>
#include <QToolButton>
#include <QToolBar>

using std::shared_ptr;

namespace sv {

class Session;

namespace processing {
class Processor;
}

namespace ui {
namespace processing {

class ProcessThreadWidget : public QMainWindow
{
	Q_OBJECT

public:
	ProcessThreadWidget(shared_ptr<Session> session, QString name,
		shared_ptr<sv::processing::Processor> processor,
		QWidget *parent = nullptr);

private:
	shared_ptr<Session> session_;
	QString name_;
	shared_ptr<sv::processing::Processor> processor_;

	QToolButton *add_button_;
	QAction *const action_add_set_value_block_;
	QAction *const action_add_get_value_block_;
	QAction *const action_add_step_block_;
	QAction *const action_add_sequence_block_;
	QAction *const action_add_wait_block_;
	QAction *const action_add_user_input_block_;
	QAction *const action_add_create_signal_block_;
	QAction *const action_remove_block_;
	QToolBar *toolbar_;
	QLineEdit *name_edit_;
	QCheckBox *run_at_startup_checkbox_;
	QListWidget *process_block_list_;

	void setup_ui();
	void setup_toolbar();

private Q_SLOTS:
	void on_action_add_set_value_block_triggered();
	void on_action_add_get_value_block_triggered();
	void on_action_add_step_block_triggered();
	void on_action_add_sequence_block_triggered();
	void on_action_add_wait_block_triggered();
	void on_action_add_user_input_block_triggered();
	void on_action_add_create_signal_block_triggered();
	void on_action_remove_block_triggered();

};

} // namespace processing
} // namespace ui
} // namespace sv

#endif // UI_PROCESSING_PROCESSTHREADWIDGET_HPP

