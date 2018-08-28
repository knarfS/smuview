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

#ifndef UI_PROCESSING_PROCESSINGWIDGET_HPP
#define UI_PROCESSING_PROCESSINGWIDGET_HPP

#include <QAction>
#include <QMainWindow>
#include <QTabWidget>
#include <QToolBar>

namespace sv {

namespace ui {

namespace processing {

class ProcessThreadWidget;

class ProcessingWidget : public QMainWindow
{
    Q_OBJECT

public:
	ProcessingWidget(QWidget *parent = nullptr);

private:
	uint thread_count_;

	QAction *const action_start_process_;
	QAction *const action_pause_process_;
	QAction *const action_stop_process_;
	QAction *const action_add_thread_;
	QAction *const action_save_process_;
	QToolBar *toolbar_;
	QTabWidget *process_tab_widget_;

	void setup_ui();
	void setup_toolbar();


private Q_SLOTS:
	void on_action_start_process_triggered();
	void on_action_pause_process_triggered();
	void on_action_stop_process_triggered();
	void on_action_add_thread_triggered();
	void on_action_save_process_triggered();

};

} // namespace processing
} // namespace ui
} // namespace sv

#endif // UI_PROCESSING_PROCESSINGWIDGET_HPP

