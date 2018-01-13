/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DIALOGS_ADDVIEWDIALOG_HPP
#define DIALOGS_ADDVIEWDIALOG_HPP

#include <memory>

#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "src/session.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
class HardwareDevice;
}

namespace views {
class BaseView;
}

namespace dialogs {

class AddViewDialog : public QDialog
{
	Q_OBJECT

public:
	AddViewDialog(const Session &session,
		const shared_ptr<devices::HardwareDevice> device,
		int selected_view_type,
		QWidget *parent = nullptr);

	shared_ptr<views::BaseView> view();

private:
	void setup_ui();
	void setup_ui_control_tab();
	void setup_ui_panel_tab();
	void setup_ui_plot_tab();
	QTreeWidget * setup_ui_channel_tree();
	void recursive_up_checks(QTreeWidgetItem *parent);
	void recursive_down_checks(QTreeWidgetItem *parent);

	const Session &session_;
	const shared_ptr<devices::HardwareDevice> device_;
	int selected_view_type_; // TODO
	shared_ptr<views::BaseView> view_;

	QTabWidget *tab_widget_;
	QTreeWidget *panel_channel_tree_;
	QTreeWidget *plot_channel_tree_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

private Q_SLOTS:
	void update_checks(QTreeWidgetItem *, int);

};

} // namespace dialogs
} // namespace sv

#endif // DIALOGS_ADDVIEWDIALOG_HPP
