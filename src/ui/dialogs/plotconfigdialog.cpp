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

#include <map>

#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QIcon>
#include <QLineEdit>
#include <QString>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "plotconfigdialog.hpp"
#include "src/widgets/plot/plot.hpp"

Q_DECLARE_METATYPE(sv::widgets::plot::PlotUpdateMode)

namespace sv {
namespace ui {
namespace dialogs {

PlotConfigDialog::PlotConfigDialog(widgets::plot::Plot *plot, QWidget *parent) :
	QDialog(parent),
	plot_(plot)
{
	setup_ui();
}

void PlotConfigDialog::setup_ui()
{
	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(mainIcon);
	this->setWindowTitle(tr("Plot Config"));
	this->setMinimumWidth(500);

	QVBoxLayout *main_layout = new QVBoxLayout();
	QFormLayout *form_layout = new QFormLayout();

	plot_update_mode_combobox_ = new QComboBox();
	size_t cb_index = 0;
	for (auto update_mode_pair : widgets::plot::plot_update_mode_name_map) {
		plot_update_mode_combobox_->addItem(
			update_mode_pair.second,
			QVariant::fromValue(update_mode_pair.first));
		if (plot_->update_mode() == update_mode_pair.first)
			plot_update_mode_combobox_->setCurrentIndex(cb_index);
		++cb_index;
	}
	connect(plot_update_mode_combobox_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_update_mode_changed()));
	form_layout->addRow(tr("Plot Mode"), plot_update_mode_combobox_);

	time_span_edit_ = new QLineEdit();
	time_span_edit_->setValidator(new QDoubleValidator);
	time_span_edit_->setText(QString("%1").arg(plot_->time_span(), 0, 'f'));
	form_layout->addRow(tr("Time Span"), time_span_edit_);

	add_time_edit_ = new QLineEdit();
	add_time_edit_->setValidator(new QDoubleValidator);
	add_time_edit_->setText(QString("%1").arg(plot_->add_time(), 0, 'f'));
	form_layout->addRow(tr("Add Time"), add_time_edit_);

	switch (plot_->update_mode()) {
	case widgets::plot::PlotUpdateMode::Additive:
		setup_ui_additive();
		break;
	case widgets::plot::PlotUpdateMode::Rolling:
		setup_ui_rolling();
		break;
	case widgets::plot::PlotUpdateMode::Oscilloscope:
		setup_ui_oscilloscope();
		break;
	}

	main_layout->addLayout(form_layout);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void PlotConfigDialog::setup_ui_additive()
{
	time_span_edit_->setDisabled(true);
	add_time_edit_->setDisabled(false);
}

void PlotConfigDialog::setup_ui_rolling()
{
	time_span_edit_->setDisabled(false);
	add_time_edit_->setDisabled(false);
}

void PlotConfigDialog::setup_ui_oscilloscope()
{
	time_span_edit_->setDisabled(false);
	add_time_edit_->setDisabled(true);
}

void PlotConfigDialog::on_update_mode_changed()
{
	QVariant update_mode_var = plot_update_mode_combobox_->currentData();
	switch (update_mode_var.value<sv::widgets::plot::PlotUpdateMode>()) {
	case widgets::plot::PlotUpdateMode::Additive:
		setup_ui_additive();
		break;
	case widgets::plot::PlotUpdateMode::Rolling:
		setup_ui_rolling();
		break;
	case widgets::plot::PlotUpdateMode::Oscilloscope:
		setup_ui_oscilloscope();
		break;
	}
}

void PlotConfigDialog::accept()
{
	QVariant update_mode_var = plot_update_mode_combobox_->currentData();
	plot_->set_update_mode(
		update_mode_var.value<sv::widgets::plot::PlotUpdateMode>());

	plot_->set_time_span(time_span_edit_->text().toDouble());
	plot_->set_add_time(add_time_edit_->text().toDouble());

	QDialog::accept();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
