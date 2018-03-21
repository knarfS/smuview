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

#ifndef DIALOGS_PLOTCONFIGDIALOG_HPP
#define DIALOGS_PLOTCONFIGDIALOG_HPP

#include <map>

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QString>
#include <QWidget>

#include "src/widgets/plot/plot.hpp"

namespace sv {

namespace dialogs {

class PlotConfigDialog : public QDialog
{
	Q_OBJECT

public:
	PlotConfigDialog(widgets::plot::Plot *plot, QWidget *parent = nullptr);

private:
	void setup_ui();
	void setup_ui_additive();
	void setup_ui_rolling();
	void setup_ui_oscilloscope();

	widgets::plot::Plot *plot_;
	QComboBox *plot_update_mode_combobox_;
	QLineEdit *time_span_edit_;
	QLineEdit *add_time_edit_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void on_update_mode_changed();
	void accept() override;

};

} // namespace dialogs
} // namespace sv

#endif // DIALOGS_PLOTCONFIGDIALOG_HPP
