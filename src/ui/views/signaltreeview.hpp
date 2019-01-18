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

#ifndef UI_VIEWS_SIGNALTREEVIEW_HPP
#define UI_VIEWS_SIGNALTREEVIEW_HPP

#include <memory>

#include <QAction>
#include <QToolBar>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace ui {

namespace devices {
class SignalTree;
}

namespace views {

class SignalTreeView : public BaseView
{
	Q_OBJECT

public:
	SignalTreeView(const Session& session, QWidget* parent = nullptr);

	QString title() const override;

private:
	void setup_ui();
	void setup_toolbar();
	void connect_signals();

	QAction *const action_add_device_;
	QAction *const action_add_user_device_;
	QAction *const action_delete_device_;
	QAction *const action_add_user_channel_;
	QAction *const action_add_math_channel_;
	QAction *const action_delete_channel_;
	QToolBar *toolbar_;
	devices::SignalTree *signal_tree_;

private Q_SLOTS:
	void update_toolbar();
	void on_action_add_device_triggered();
	void on_action_add_user_device_triggered();
	void on_action_delete_device_triggered();
	void on_action_add_math_channel_triggered();
	void on_action_delete_channel_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_SIGNALTREEVIEW_HPP
