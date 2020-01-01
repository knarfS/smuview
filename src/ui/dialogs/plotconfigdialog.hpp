/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DIALOGS_PLOTCONFIGDIALOG_HPP
#define UI_DIALOGS_PLOTCONFIGDIALOG_HPP

#include <map>

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QString>
#include <QTabWidget>
#include <QWidget>

#include "src/ui/widgets/plot/plot.hpp"
#include "src/ui/views/plotview.hpp"

namespace sv {
namespace ui {
namespace dialogs {

class PlotConfigDialog : public QDialog
{
	Q_OBJECT

public:
	PlotConfigDialog(widgets::plot::Plot *plot, views::PlotType plot_type,
		QWidget *parent = nullptr);

private:
	void setup_ui();
	void setup_ui_plot_mode_tab();
	void setup_ui_markers_tab();
	void setup_ui_style_tab();
	void setup_ui_additive();
	void setup_ui_rolling();
	void setup_ui_oscilloscope();

	widgets::plot::Plot *plot_;
	views::PlotType plot_type_;
	QTabWidget *tab_widget_;
	QComboBox *plot_update_mode_combobox_;
	QLineEdit *time_span_edit_;
	QLineEdit *add_time_edit_;
	QComboBox *markers_box_pos_combobox_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void on_update_mode_changed();
	void accept() override;

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_PLOTCONFIGDIALOG_HPP
