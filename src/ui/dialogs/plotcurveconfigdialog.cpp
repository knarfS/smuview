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

#include <QCheckBox>
#include <QBrush>
#include <QDebug>
#include <QFormLayout>
#include <QIcon>
#include <QPen>
#include <QSize>
#include <QWidget>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include "plotcurveconfigdialog.hpp"
#include "src/ui/widgets/colorbutton.hpp"

namespace sv {
namespace ui {
namespace dialogs {

PlotCurveConfigDialog::PlotCurveConfigDialog(QwtPlotCurve *plot_curve,
		QWidget *parent) :
	QDialog(parent),
	plot_curve_(plot_curve)
{
	setup_ui();
}

void PlotCurveConfigDialog::setup_ui()
{
	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(mainIcon);
	this->setWindowTitle(tr("Plot Config"));
	this->setMinimumWidth(500);

	QFormLayout *main_layout = new QFormLayout;

	visible_checkbox_ = new QCheckBox();
	visible_checkbox_->setChecked(plot_curve_->isVisible());
	main_layout->addRow(tr("Visible"), visible_checkbox_);

	color_button_ = new widgets::ColorButton();
	color_button_->set_color(plot_curve_->pen().color());
	main_layout->addRow(tr("Color"), color_button_);

	sample_points_checkbox_ = new QCheckBox();
	sample_points_checkbox_->setChecked(plot_curve_->symbol() != NULL);
	main_layout->addRow(tr("Show sample points"), sample_points_checkbox_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void PlotCurveConfigDialog::accept()
{
	plot_curve_->setVisible(visible_checkbox_->isChecked());

	QPen pen = plot_curve_->pen();
	pen.setColor(color_button_->color());
	plot_curve_->setPen(pen);

	QwtSymbol *symbol = NULL;
	if (sample_points_checkbox_->isChecked()) {
		symbol = new QwtSymbol(QwtSymbol::Ellipse);
		symbol->setBrush(QBrush(color_button_->color()));
		symbol->setPen(QPen(color_button_->color(), 2));
		symbol->setSize(QSize(3, 3));
	}
	plot_curve_->setSymbol(symbol);

	QDialog::accept();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
