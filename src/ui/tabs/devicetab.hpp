/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_TABS_DEVICETAB_HPP
#define UI_TABS_DEVICETAB_HPP

#include <memory>

#include <QAction>
#include <QCloseEvent>
#include <QToolBar>
#include <QWidget>

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
		shared_ptr<sv::devices::BaseDevice> device, QWidget *parent = nullptr);

	QString title() override;
	bool request_close() override;
	virtual void clear_signals();

protected:
	void save_settings() const override;
	void restore_settings() override;

	shared_ptr<sv::devices::BaseDevice> device_;
	util::TimeUnit time_unit_;

private:
	void setup_toolbar();

	QAction *const action_aquire_;
	QAction *const action_save_as_;
	QAction *const action_add_control_view_;
	QAction *const action_add_panel_view_;
	QAction *const action_add_plot_view_;
	QAction *const action_add_table_view_;
	QAction *const action_add_math_channel_;
	QAction *const action_about_;
	QToolBar *toolbar_;

public Q_SLOTS:

private Q_SLOTS:
	void on_action_aquire_triggered();
	void on_action_save_as_triggered();
	void on_action_add_control_view_triggered();
	void on_action_add_panel_view_triggered();
	void on_action_add_plot_view_triggered();
	void on_action_add_table_view_triggered();
	void on_action_add_math_channel_triggered();
	void on_action_about_triggered();

};

} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_DEVICETAB_HPP
