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

#ifndef UI_VIEWS_SMUSCRIPTVIEW_HPP
#define UI_VIEWS_SMUSCRIPTVIEW_HPP

#include <memory>

#include <QAction>
#include <QToolBar>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace python {
class SmuScript;
}

namespace ui {
namespace views {

class SmuScriptView : public BaseView
{
	Q_OBJECT

public:
	SmuScriptView(Session &session, QWidget *parent = nullptr);

	QString title() const;

private:
	QAction *const action_start_script_;
	QToolBar *toolbar_;
	shared_ptr<python::SmuScript> smu_script_; // TODO: Use unique_ptr instead?

	void setup_ui();
	void setup_toolbar();
	void connect_signals();

private Q_SLOTS:
	void on_action_start_script_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_SMUSCRIPTVIEW_HPP
