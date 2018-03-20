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
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QLineEdit>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QWidget>
#include <qwt_scale_engine.h>

#include "axispopup.hpp"
#include <src/widgets/popup.hpp>

namespace sv {
namespace widgets {
namespace plot {

AxisPopup::AxisPopup(Plot *plot, int axis_id, QWidget *parent) :
	Popup(parent),
	plot_(plot),
	axis_id_(axis_id)
{
	setup_ui();
}


void AxisPopup::setup_ui()
{
	QVBoxLayout *main_layout = new QVBoxLayout;
	QFormLayout *form_layout = new QFormLayout;

	double min_value = plot_->axisScaleDiv(axis_id_).lowerBound();
	axis_min_edit_ = new QLineEdit();
	axis_min_edit_->setValidator(new QDoubleValidator);
	axis_min_edit_->setText(QString("%1").arg(min_value, 0, 'f'));
	form_layout->addRow(tr("Min"), axis_min_edit_);

	double max_value = plot_->axisScaleDiv(axis_id_).upperBound();
	axis_max_edit_ = new QLineEdit();
	axis_max_edit_->setValidator(new QDoubleValidator);
	axis_max_edit_->setText(QString("%1").arg(max_value, 0, 'f'));
	form_layout->addRow(tr("Max"), axis_max_edit_);

	bool is_log_scale = false;
	if (dynamic_cast<QwtLogScaleEngine *>(plot_->axisScaleEngine(axis_id_)))
		is_log_scale = true;
	axis_log_check_ = new QCheckBox();
	axis_log_check_->setChecked(is_log_scale);
	form_layout->addRow(tr("Logarithmic"), axis_log_check_);

	main_layout->addLayout(form_layout);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(on_accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(close()));

	this->setLayout(main_layout);
}

void AxisPopup::showEvent(QShowEvent *event)
{
	widgets::Popup::showEvent(event);
}

void AxisPopup::on_accept()
{
	plot_->setAxisScale(axis_id_,
		axis_min_edit_->text().toDouble(), axis_max_edit_->text().toDouble());

	if (axis_log_check_->isChecked())
		plot_->setAxisScaleEngine(axis_id_, new QwtLogScaleEngine);
	else
		plot_->setAxisScaleEngine(axis_id_, new QwtLinearScaleEngine);

	plot_->replot();
	this->close();
}

} // namespace plot
} // namespace widgets
} // namespace sv
