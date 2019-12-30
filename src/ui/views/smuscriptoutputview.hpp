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

#ifndef UI_VIEWS_SMUSCRIPTOUTPUTVIEW_HPP
#define UI_VIEWS_SMUSCRIPTOUTPUTVIEW_HPP

#include <string>

#include <QAction>
#include <QPlainTextEdit>
#include <QString>
#include <QToolBar>

#include "src/ui/views/baseview.hpp"

namespace sv {

class Session;

namespace ui {
namespace views {

class SmuScriptOutputView : public BaseView
{
	Q_OBJECT

public:
	SmuScriptOutputView(Session& session, QWidget* parent = nullptr);

	QString title() const;

private:
	bool auto_scroll_;
	QAction *const action_auto_scroll_;
	QAction *const action_clear_output_;
	QToolBar *toolbar_;
	QPlainTextEdit *output_edit_;

	void setup_ui();
	void setup_toolbar();
	void scroll_to_bottom();

public Q_SLOTS:
	void append_out_text(const std::string &text);
	void append_err_text(const std::string &text);

private Q_SLOTS:
	void on_action_auto_scroll_triggered();
	void on_action_clear_output_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_SMUSCRIPTOUTPUTVIEW_HPP
