/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>

#include <QCheckBox>
#include <QComboBox>
#include <QBrush>
#include <QDebug>
#include <QFormLayout>
#include <QIcon>
#include <QLineEdit>
#include <QPen>
#include <QPushButton>
#include <QSize>
#include <QWidget>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include "plotcurveconfigdialog.hpp"
#include "src/ui/widgets/plot/curve.hpp"
#include "src/ui/widgets/plot/plot.hpp"
#include "src/ui/widgets/colorbutton.hpp"

Q_DECLARE_METATYPE(Qt::PenStyle)
Q_DECLARE_METATYPE(QwtSymbol::Style)

namespace sv {
namespace ui {
namespace dialogs {

PlotCurveConfigDialog::PlotCurveConfigDialog(widgets::plot::Curve *curve,
		widgets::plot::Plot *plot, QWidget *parent) :
	QDialog(parent),
	curve_(curve),
	plot_(plot)
{
	assert(curve_);
	assert(plot_);

	setup_ui();
}

void PlotCurveConfigDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Curve Config"));
	this->setMinimumWidth(500);

	QFormLayout *main_layout = new QFormLayout;

	name_edit_ = new QLineEdit();
	name_edit_->setText(curve_->name());
	main_layout->addRow(tr("Name"), name_edit_);

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
	QPushButton *remove_button = new QPushButton(
		QIcon::fromTheme("edit-delete", QIcon(":/icons/edit-delete.png")),
		tr("Remove Curve"));
	button_box_->addButton(remove_button, QDialogButtonBox::DestructiveRole);
	main_layout->addWidget(button_box_);
	connect(button_box_, &QDialogButtonBox::accepted,
		this, &PlotCurveConfigDialog::accept);
	connect(button_box_, &QDialogButtonBox::rejected,
		this, &PlotCurveConfigDialog::reject);
	connect(remove_button, &QPushButton::clicked,
		this, &PlotCurveConfigDialog::remove_curve);

	this->setLayout(main_layout);
}

void PlotCurveConfigDialog::accept()
{
	curve_->set_name(name_edit_->text());
	curve_->plot_curve()->setVisible(visible_checkbox_->isChecked());
	curve_->set_color(color_button_->color());
	curve_->set_style(line_type_box_->currentData().value<Qt::PenStyle>());
	curve_->set_symbol(symbol_type_box_->currentData().value<QwtSymbol::Style>());

	QDialog::accept();
}

void PlotCurveConfigDialog::remove_curve()
{
	plot_->remove_curve(curve_);
	QDialog::close();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
