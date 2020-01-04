/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DIALOGS_ADDVIEWDIALOG_HPP
#define UI_DIALOGS_ADDVIEWDIALOG_HPP

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

namespace ui {

namespace devices {
class SelectConfigurableForm;
class SelectPropertyForm;
class SelectSignalWidget;
namespace devicetree {
class DeviceTreeView;
}
}
namespace views {
class BaseView;
}

namespace dialogs {

class AddViewDialog : public QDialog
{
	Q_OBJECT

public:
	AddViewDialog(Session &session,
		const shared_ptr<sv::devices::BaseDevice> device,
		int selected_tab,
		QWidget *parent = nullptr);

	vector<ui::views::BaseView *> views();

private:
	void setup_ui();
	void setup_ui_control_tab();
	void setup_ui_sequence_tab();
	void setup_ui_panel_tab();
	void setup_ui_time_plot_tab();
	void setup_ui_xy_plot_tab();
	void setup_ui_table_tab();

	Session &session_;
	const shared_ptr<sv::devices::BaseDevice> device_;
	int selected_tab_;
	vector<ui::views::BaseView *> views_;

	QTabWidget *tab_widget_;
	ui::devices::SelectConfigurableForm *configurable_configurable_form_;
	ui::devices::SelectPropertyForm *sequence_property_form_;
	ui::devices::devicetree::DeviceTreeView *panel_channel_tree_;
	ui::devices::devicetree::DeviceTreeView *time_plot_channel_tree_;
	ui::devices::SelectSignalWidget *xy_plot_x_signal_widget_;
	ui::devices::SelectSignalWidget *xy_plot_y_signal_widget_;
	ui::devices::devicetree::DeviceTreeView *table_signal_tree_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_ADDVIEWDIALOG_HPP
