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

#ifndef UI_VIEWS_SMUSCRIPTTREEVIEW_HPP
#define UI_VIEWS_SMUSCRIPTTREEVIEW_HPP

#include <QAction>
#include <QFileSystemModel>
#include <QModelIndex>
#include <QSettings>
#include <QString>
#include <QToolBar>
#include <QTreeView>
#include <QUuid>

#include "src/ui/views/baseview.hpp"

namespace sv {

class Session;

namespace ui {
namespace views {

class SmuScriptTreeView : public BaseView
{
	Q_OBJECT

public:
	SmuScriptTreeView(Session &session, QUuid uuid = QUuid(),
		QWidget *parent = nullptr);

	QString title() const override;

	void save_settings(QSettings &settings) const override;
	void restore_settings(QSettings &settings) override;

private:
	QAction *const action_new_script_;
	QAction *const action_open_script_;
	QAction *const action_run_script_;
	QString script_dir_;
	QToolBar *toolbar_;
	QFileSystemModel *file_system_model_;
	QTreeView *file_system_tree_;

	void setup_ui();
	void setup_toolbar();
	void connect_signals();
	void scroll_to_script_dir();
	void open_script_file(const QModelIndex& index);

private Q_SLOTS:
	void on_action_new_script_triggered();
	void on_action_open_script_triggered();
	void on_action_run_script_triggered();
	void on_tree_double_clicked(const QModelIndex& index);
	void on_script_started();
	void on_script_finished();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_SMUSCRIPTTREEVIEW_HPP
