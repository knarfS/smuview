/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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
#include <QDebug>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QWidget>
#include <Node>

#include "rampsourcepopup.hpp"
#include "src/flow/rampsourcedatamodel.hpp"
#include "src/ui/widgets/popup.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace flow {

RampSourcePopup::RampSourcePopup(QtNodes::Node &node, QWidget *parent) :
	Popup(parent),
	node_(node)
{
	setup_ui();
}


void RampSourcePopup::setup_ui()
{
	sv::flow::RampSourceDataModel *data_model =
		static_cast<sv::flow::RampSourceDataModel *>(node_.nodeDataModel());

	QVBoxLayout *main_layout = new QVBoxLayout();
	QFormLayout *form_layout = new QFormLayout();

	start_value_box_ = new QDoubleSpinBox();
	start_value_box_->setValue(data_model->start_value());
	form_layout->addRow(tr("Start value"), start_value_box_);

	end_value_box_ = new QDoubleSpinBox();
	end_value_box_->setValue(data_model->end_value());
	form_layout->addRow(tr("End value"), end_value_box_);

	step_width_box_ = new QDoubleSpinBox();
	step_width_box_->setValue(data_model->step_width());
	form_layout->addRow(tr("Step width"), step_width_box_);

	step_trigger_box_ = new QCheckBox();
	step_trigger_box_->setChecked(data_model->step_trigger());
	form_layout->addRow(tr("Step trigger"), step_trigger_box_);

	// TODO: disable when step_trigger is true
	step_delay_box_ = new QSpinBox();
	step_delay_box_->setSuffix(QString(" %1").arg("ms"));
	step_delay_box_->setMinimum(0);
	step_delay_box_->setMaximum(600000);
	step_delay_box_->setValue(data_model->step_delay());
	form_layout->addRow(tr("Step delay"), step_delay_box_);

	main_layout->addLayout(form_layout);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(on_accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(close()));

	this->setLayout(main_layout);
}

void RampSourcePopup::showEvent(QShowEvent *event)
{
	widgets::Popup::showEvent(event);
}

void RampSourcePopup::on_accept()
{
	sv::flow::RampSourceDataModel *data_model =
		static_cast<sv::flow::RampSourceDataModel *>(node_.nodeDataModel());

	data_model->set_start_value(start_value_box_->value());
	data_model->set_end_value(end_value_box_->value());
	data_model->set_step_width(step_width_box_->value());
	data_model->set_step_trigger(step_trigger_box_->isChecked());
	data_model->set_step_delay(step_delay_box_->value());

	this->close();
}

} // namespace flow
} // namespace widgets
} // namespace ui
} // namespace sv
