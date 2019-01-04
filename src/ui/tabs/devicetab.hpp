/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_TABS_DEVICETAB_HPP
#define UI_TABS_DEVICETAB_HPP

#include <memory>

#include <QAction>
#include <QMainWindow>
#include <QToolBar>

#include "src/util.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/tabs/basetab.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace ui {
namespace tabs {

class DeviceTab : public BaseTab
{
	Q_OBJECT

private:

public:
	DeviceTab(Session &session,
		shared_ptr<sv::devices::BaseDevice> device, QMainWindow *parent);

	virtual void clear_signals();

protected:
	shared_ptr<sv::devices::BaseDevice> device_;
	util::TimeUnit time_unit_;

private:
	void setup_toolbar();

	QAction *const action_open_;
	QAction *const action_save_as_;
	QAction *const action_add_control_view_;
	QAction *const action_add_panel_view_;
	QAction *const action_add_plot_view_;
	QAction *const action_add_math_channel_;
	QAction *const action_add_flow_;
	QAction *const action_reset_data_;
	QAction *const action_about_;
	QToolBar *toolbar;

public Q_SLOTS:

private Q_SLOTS:
	void on_action_open_triggered();
	void on_action_save_as_triggered();
	void on_action_add_control_view_triggered();
	void on_action_add_panel_view_triggered();
	void on_action_add_plot_view_triggered();
	void on_action_add_math_channel_triggered();
	void on_action_add_flow_triggered();
	void on_action_reset_data_triggered();
	void on_action_about_triggered();

};

} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_DEVICETAB_HPP
