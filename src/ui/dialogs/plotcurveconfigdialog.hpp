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

#ifndef UI_DIALOGS_PLOTCURVECONFIGDIALOG_HPP
#define UI_DIALOGS_PLOTCURVECONFIGDIALOG_HPP

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QWidget>
#include <qwt_plot_curve.h>

namespace sv {
namespace ui {

namespace widgets {
class ColorButton;
}

namespace dialogs {

class PlotCurveConfigDialog : public QDialog
{
	Q_OBJECT

public:
	PlotCurveConfigDialog(QwtPlotCurve *plot_curve, QWidget *parent = nullptr);

private:
	void setup_ui();

	QwtPlotCurve *plot_curve_;
	QCheckBox *visible_checkbox_;
	widgets::ColorButton *color_button_;
	QCheckBox *sample_points_checkbox_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_PLOTCURVECONFIGDIALOG_HPP
