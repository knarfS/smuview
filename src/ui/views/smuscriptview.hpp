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

#ifndef UI_VIEWS_SMUSCRIPTVIEW_HPP
#define UI_VIEWS_SMUSCRIPTVIEW_HPP

#include <string>

#include <QAction>
#include <QSettings>
#include <QToolBar>
#include <QUuid>

#include <QCodeEditor>

#include "src/ui/views/baseview.hpp"

using std::string;

namespace sv {

class Session;

namespace ui {
namespace views {

class SmuScriptView : public BaseView
{
	Q_OBJECT

public:
	SmuScriptView(Session& session, QUuid uuid = QUuid(),
		QWidget* parent = nullptr);

	QString title() const override;
	void load_file(const string &file_name);
	bool ask_to_save(const QString &title);

	void save_settings(QSettings &settings) const override;
	void restore_settings(QSettings &settings) override;

private:
	string script_file_name_;
	QAction *const action_open_;
	QAction *const action_save_;
	QAction *const action_save_as_;
	QAction *const action_run_;
	QToolBar *toolbar_;
	QCodeEditor *editor_;
	bool text_changed_;
	bool started_from_here_;

	void setup_ui();
	void setup_toolbar();
	void connect_signals();
	bool save(QString file_name);

public Q_SLOTS:
	void run_script();
	void stop_script();

private Q_SLOTS:
	void on_action_open_triggered();
	void on_action_save_triggered();
	void on_action_save_as_triggered();
	void on_action_run_triggered();
	void on_text_changed();
	void on_script_started();
	void on_script_finished();

Q_SIGNALS:
	void file_name_changed(const QString &file_name);
	void file_save_state_changed(bool is_unsaved);
	void script_started();
	void script_finished();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_SMUSCRIPTVIEW_HPP
