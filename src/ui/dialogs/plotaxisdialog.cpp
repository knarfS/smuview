/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <QBoxLayout>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QWidget>
#include <qwt_scale_engine.h>

#include "src/ui/dialogs/plotaxisdialog.hpp"

namespace sv {
namespace ui {
namespace dialogs {

PlotAxisDialog::PlotAxisDialog(widgets::plot::Plot *plot, int axis_id, QWidget *parent) :
	QDialog(parent),
	plot_(plot),
	axis_id_(axis_id)
{
	setup_ui();
}


void PlotAxisDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Plot %1 Axis Config").arg(plot_->axisTitle(axis_id_).text()));
	this->setMinimumWidth(300);

	QVBoxLayout *main_layout = new QVBoxLayout;

	QFormLayout *form_layout = new QFormLayout;
	main_layout->addLayout(form_layout);

	// Lower boundary
	double lower_value = plot_->axisScaleDiv(axis_id_).lowerBound();
	axis_lower_edit_ = new QLineEdit();
	axis_lower_edit_->setValidator(new QDoubleValidator());
	axis_lower_edit_->setText(QString("%1").arg(lower_value, 0, 'f'));
	QString lower_label;
	if (axis_id_ == QwtPlot::xTop  || axis_id_ == QwtPlot::xBottom)
		lower_label = tr("Left boundary");
	else
		lower_label = tr("Bottom boundary");

	axis_lower_locked_check_ = new QCheckBox(tr("Locked"));
	axis_lower_locked_check_->setChecked(plot_->is_axis_locked(
		axis_id_, widgets::plot::AxisBoundary::LowerBoundary));

	QHBoxLayout *lower_layout = new QHBoxLayout;
	lower_layout->addWidget(axis_lower_edit_);
	lower_layout->addSpacing(15);
	lower_layout->addWidget(axis_lower_locked_check_);

	// Upper boundary
	double upper_value = plot_->axisScaleDiv(axis_id_).upperBound();
	axis_upper_edit_ = new QLineEdit();
	axis_upper_edit_->setValidator(new QDoubleValidator());
	axis_upper_edit_->setText(QString("%1").arg(upper_value, 0, 'f'));
	QString upper_label;
	if (axis_id_ == QwtPlot::xTop  || axis_id_ == QwtPlot::xBottom)
		upper_label = tr("Right boundary");
	else
		upper_label = tr("Top boundary");

	axis_upper_locked_check_ = new QCheckBox(tr("Locked"));
	axis_upper_locked_check_->setChecked(plot_->is_axis_locked(
		axis_id_, widgets::plot::AxisBoundary::UpperBoundary));

	QHBoxLayout *upper_layout = new QHBoxLayout;
	upper_layout->addWidget(axis_upper_edit_);
	upper_layout->addSpacing(15);
	upper_layout->addWidget(axis_upper_locked_check_);

	if (axis_id_ == QwtPlot::xTop  || axis_id_ == QwtPlot::xBottom) {
		form_layout->addRow(lower_label, lower_layout);
		form_layout->addRow(upper_label, upper_layout);
	}
	else {
		// Reverse the display order for y axes
		form_layout->addRow(upper_label, upper_layout);
		form_layout->addRow(lower_label, lower_layout);
	}

	bool is_log_scale = false;
	if (dynamic_cast<QwtLogScaleEngine *>(plot_->axisScaleEngine(axis_id_)))
		is_log_scale = true;
	axis_log_check_ = new QCheckBox();
	axis_log_check_->setChecked(is_log_scale);
	form_layout->addRow(tr("Logarithmic scale"), axis_log_check_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, &QDialogButtonBox::accepted,
		this, &PlotAxisDialog::accept);
	connect(button_box_, &QDialogButtonBox::rejected,
		this, &PlotAxisDialog::reject);

	this->setLayout(main_layout);
}

void PlotAxisDialog::accept()
{
	plot_->setAxisScale(axis_id_,
		axis_lower_edit_->text().toDouble(),
		axis_upper_edit_->text().toDouble());

	plot_->set_axis_locked(axis_id_, widgets::plot::AxisBoundary::LowerBoundary,
		axis_lower_locked_check_->isChecked());
	plot_->set_axis_locked(axis_id_, widgets::plot::AxisBoundary::UpperBoundary,
		axis_upper_locked_check_->isChecked());

	if (axis_log_check_->isChecked())
		plot_->setAxisScaleEngine(axis_id_, new QwtLogScaleEngine);
	else
		plot_->setAxisScaleEngine(axis_id_, new QwtLinearScaleEngine);

	plot_->replot();

	QDialog::accept();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
