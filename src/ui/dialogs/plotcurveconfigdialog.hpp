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

#ifndef UI_DIALOGS_PLOTCURVECONFIGDIALOG_HPP
#define UI_DIALOGS_PLOTCURVECONFIGDIALOG_HPP

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QWidget>

namespace sv {
namespace ui {

namespace widgets {
class ColorButton;
namespace plot {
class Curve;
}
}

namespace dialogs {

class PlotCurveConfigDialog : public QDialog
{
	Q_OBJECT

public:
	PlotCurveConfigDialog(widgets::plot::Curve *curve,
		QWidget *parent = nullptr);

private:
	void setup_ui();

	widgets::plot::Curve *curve_;
	QCheckBox *visible_checkbox_;
	widgets::ColorButton *color_button_;
	QComboBox *line_type_box_;
	QComboBox *symbol_type_box_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_PLOTCURVECONFIGDIALOG_HPP
