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

#include "src/session.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {

namespace devices {
class BaseDevice;
}

namespace views {
class BaseView;
}

namespace widgets {
class SignalTree;
}

namespace dialogs {

class AddViewDialog : public QDialog
{
	Q_OBJECT

public:
	AddViewDialog(const Session &session,
		const shared_ptr<devices::BaseDevice> device,
		int selected_view_type,
		QWidget *parent = nullptr);

	vector<views::BaseView *> views();

private:
	void setup_ui();
	void setup_ui_control_tab();
	void setup_ui_panel_tab();
	void setup_ui_time_plot_tab();
	void setup_ui_xy_plot_tab();
	void setup_ui_table_tab();

	const Session &session_;
	const shared_ptr<devices::BaseDevice> device_;
	int selected_view_type_; // TODO
	vector<views::BaseView *> views_;

	QTabWidget *tab_widget_;
	widgets::SignalTree *panel_channel_tree_;
	widgets::SignalTree *time_plot_channel_tree_;
	widgets::SignalTree *xy_plot_x_signal_tree_;
	widgets::SignalTree *xy_plot_y_signal_tree_;
	widgets::SignalTree *table_signal_tree_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

};

} // namespace dialogs
} // namespace sv

#endif // DIALOGS_ADDVIEWDIALOG_HPP
