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

#ifndef UI_VIEWS_DEVICETREEVIEW_HPP
#define UI_VIEWS_DEVICETREEVIEW_HPP

#include <memory>

#include <QAction>
#include <QToolBar>

#include "src/ui/devices/devicetree/devicetreemodel.hpp"
#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace ui {

namespace devices {
namespace devicetree {
class DeviceTreeView;
}
}

namespace views {

class DeviceTreeView : public BaseView
{
	Q_OBJECT

public:
	DeviceTreeView(Session &session,
		QWidget *parent = nullptr);

	QString title() const;

private:
	QAction *const action_add_device_;
	QAction *const action_add_virtualdevice_;
	QAction *const action_disconnect_device_;
	QToolBar *toolbar_;
	devices::devicetree::DeviceTreeView  *device_tree_;

	void setup_ui();
	void setup_toolbar();
	void connect_signals();

private Q_SLOTS:
	void on_action_add_device_triggered();
	void on_action_add_virtualdevice_triggered();
	void on_action_disconnect_device_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_DEVICETREEVIEW_HPP
