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

#include <QAction>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QString>
#include <QToolBar>

namespace sv {

namespace ui {

namespace processing {

class ProcessThreadWidget : public QMainWindow
{
    Q_OBJECT

public:
	ProcessThreadWidget(QString name, QWidget *parent = nullptr);

private:
	QString name_;

	QAction *const action_add_block_;
	QToolBar *toolbar_;
	QLineEdit *name_edit_;
	QCheckBox *run_at_startup_checkbox_;
	QListWidget *process_block_list_;

	void setup_ui();
	void setup_toolbar();


private Q_SLOTS:
	void on_action_add_block_triggered();

};

} // namespace processing
} // namespace ui
} // namespace sv

#endif // UI_PROCESSING_PROCESSTHREADWIDGET_HPP

