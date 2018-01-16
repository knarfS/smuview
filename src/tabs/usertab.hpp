/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef TABS_USERTAB_HPP
#define TABS_USERTAB_HPP

#include <QAction>
#include <QMainWindow>
#include <QToolBar>

#include "src/tabs/basetab.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace tabs {

class UserTab : public BaseTab
{
	Q_OBJECT

private:

public:
	UserTab(Session &session, QMainWindow *parent);

protected:

private:
	void setup_toolbar();

	QAction *const action_add_control_view_;
	QAction *const action_add_panel_view_;
	QAction *const action_add_plot_view_;
	QToolBar *toolbar;

public Q_SLOTS:

private Q_SLOTS:
	void on_action_add_control_view_triggered();
	void on_action_add_panel_view_triggered();
	void on_action_add_plot_view_triggered();

};

} // namespace tabs
} // namespace sv

#endif // TABS_USERTAB_HPP
