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

#include <QCheckBox>
#include <QComboBox>
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
#include "src/ui/widgets/plot/curve.hpp"
#include "src/ui/widgets/colorbutton.hpp"

Q_DECLARE_METATYPE(Qt::PenStyle)
Q_DECLARE_METATYPE(QwtSymbol::Style)

namespace sv {
namespace ui {
namespace dialogs {

PlotCurveConfigDialog::PlotCurveConfigDialog(widgets::plot::Curve *curve,
		QWidget *parent) :
	QDialog(parent),
	curve_(curve)
{
	setup_ui();
}

void PlotCurveConfigDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Plot Config"));
	this->setMinimumWidth(500);

	QFormLayout *main_layout = new QFormLayout;

	visible_checkbox_ = new QCheckBox();
	visible_checkbox_->setChecked(curve_->plot_curve()->isVisible());
	main_layout->addRow(tr("Visible"), visible_checkbox_);

	color_button_ = new widgets::ColorButton();
	color_button_->set_color(curve_->color());
	main_layout->addRow(tr("Color"), color_button_);

	line_type_box_ = new QComboBox();
	line_type_box_->addItem(tr("None"), QVariant::fromValue(Qt::NoPen));
	line_type_box_->addItem(tr("Solid"), QVariant::fromValue(Qt::SolidLine));
	line_type_box_->addItem(tr("Dots"), QVariant::fromValue(Qt::DotLine));
	line_type_box_->addItem(tr("Dashes"), QVariant::fromValue(Qt::DashLine));
	for (int i=0; i<line_type_box_->count(); ++i)  {
		if (line_type_box_->itemData(i).value<Qt::PenStyle>() ==
				curve_->style()) {
			line_type_box_->setCurrentIndex(i);
			break;
		}
	}
	main_layout->addRow(tr("Line type"), line_type_box_);

	symbol_type_box_ = new QComboBox();
	symbol_type_box_->addItem(tr("None"), QwtSymbol::NoSymbol);
	symbol_type_box_->addItem(tr("Dot"), QwtSymbol::Ellipse);
	symbol_type_box_->addItem(tr("Cross"), QwtSymbol::XCross);
	for (int i=0; i<line_type_box_->count(); ++i)  {
		if (symbol_type_box_->itemData(i).value<QwtSymbol::Style>() ==
				curve_->symbol()) {
			symbol_type_box_->setCurrentIndex(i);
			break;
		}
	}
	main_layout->addRow(tr("Symbol type"), symbol_type_box_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void PlotCurveConfigDialog::accept()
{
	curve_->plot_curve()->setVisible(visible_checkbox_->isChecked());
	curve_->set_color(color_button_->color());
	curve_->set_style(line_type_box_->currentData().value<Qt::PenStyle>());
	curve_->set_symbol(symbol_type_box_->currentData().value<QwtSymbol::Style>());

	QDialog::accept();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
