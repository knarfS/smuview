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

#include <map>

#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QFormLayout>
#include <QIcon>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>
#include <qwt_plot_marker.h>

#include "plotdiffmarkerdialog.hpp"
#include "src/ui/widgets/plot/plot.hpp"

Q_DECLARE_METATYPE(QwtPlotMarker *)

namespace sv {
namespace ui {
namespace dialogs {

PlotDiffMarkerDialog::PlotDiffMarkerDialog(
		widgets::plot::Plot *plot, QWidget *parent) :
	QDialog(parent),
	plot_(plot)
{
	setup_ui();
}

void PlotDiffMarkerDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Plot Diff Merker"));
	this->setMinimumWidth(250);

	QVBoxLayout *main_layout = new QVBoxLayout();
	QFormLayout *form_layout = new QFormLayout();

	marker_1_combobox_ = new QComboBox();
	for (const auto &mc_pair : plot_->marker_curve_map()) {
		marker_1_combobox_->addItem(
			mc_pair.first->title().text(), QVariant::fromValue(mc_pair.first));
	}
	if (!plot_->marker_curve_map().empty())
		marker_1_combobox_->setCurrentIndex(0);
	form_layout->addRow(tr("Marker 1"), marker_1_combobox_);

	marker_2_combobox_ = new QComboBox();
	for (const auto &mc_pair : plot_->marker_curve_map()) {
		marker_2_combobox_->addItem(
			mc_pair.first->title().text(), QVariant::fromValue(mc_pair.first));
	}
	if (plot_->marker_curve_map().size() >= 2)
		marker_2_combobox_->setCurrentIndex(1);
	else if (!plot_->marker_curve_map().empty())
		marker_2_combobox_->setCurrentIndex(0);
	form_layout->addRow(tr("Marker 2"), marker_2_combobox_);

	main_layout->addLayout(form_layout);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void PlotDiffMarkerDialog::accept()
{
	QVariant marker_1_var = marker_1_combobox_->currentData();
	QVariant marker_2_var = marker_2_combobox_->currentData();
	plot_->add_diff_marker(
		marker_1_var.value<QwtPlotMarker *>(),
		marker_2_var.value<QwtPlotMarker *>());

	QDialog::accept();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
