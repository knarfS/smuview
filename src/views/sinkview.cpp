/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QPushButton>
#include <QHBoxLayout>
#include <QGroupBox>

#include "sinkview.hpp"
#include "src/data/analogsegment.hpp"
#include "src/widgets/controlbutton.hpp"
#include "src/widgets/lcddisplay.hpp"
#include "src/widgets/powerpanel.hpp"
#include "src/widgets/valuecontrol.hpp"

using std::shared_ptr;

namespace sv {
namespace views {

SinkView::SinkView(shared_ptr<devices::HardwareDevice> device,
		QWidget *parent) :
	QWidget(parent),
	device_(device)
{
	setup_ui();

	init_values();

	// Current limit control
	connect(setValueControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_value_changed(const double)));
	connect(setEnableButton, SIGNAL(state_changed(bool)),
		this, SLOT(on_enabled_changed(const bool)));

	connect(device_.get(), SIGNAL(current_limit_changed(const double)),
		setValueControl, SLOT(change_value(const double)));
}


void SinkView::init_values()
{
	setValueControl->on_value_changed(device_->get_current_limit());
	setEnableButton->on_state_changed(device_->get_enabled());
}

void SinkView::setup_ui()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	// Group Box for Setting
	QGroupBox *setValuesGroupBox = new QGroupBox(this);
	setValuesGroupBox->setTitle("Set Values");

	QHBoxLayout *setHLayout = new QHBoxLayout(this);
	QVBoxLayout *setValuesVLayout = new QVBoxLayout(this);

	double min;
	double max;
	double step;
	device_->list_current_limit(min, max, step);
	setValueControl = new widgets::ValueControl(5, "A", min, max, step, this);
	setValuesVLayout->addWidget(setValueControl);
	setValuesVLayout->addStretch(5);
	setHLayout->addItem(setValuesVLayout);

	// Enable button
	QVBoxLayout *setOptionsVLayout = new QVBoxLayout(this);
	setEnableButton = new widgets::ControlButton(
		device_->is_enable_getable(), device_->is_enable_setable(), this);
	setOptionsVLayout->addWidget(setEnableButton);
	setHLayout->addItem(setOptionsVLayout);

	setValuesGroupBox->setLayout(setHLayout);

	// Group Box for Values
	QGroupBox *getValuesGroupBox = new QGroupBox(this);
	getValuesGroupBox->setTitle("Actual Values");
	QVBoxLayout *getValuesVLayout = new QVBoxLayout(this);

	// Power panel
	powerPanel = new widgets::PowerPanel(device_, this);
	getValuesVLayout->addWidget(powerPanel);
	getValuesVLayout->addStretch(5);

	getValuesGroupBox->setLayout(getValuesVLayout);

	QHBoxLayout *valueLayout = new QHBoxLayout(this);
	valueLayout->addWidget(setValuesGroupBox);
	valueLayout->addWidget(getValuesGroupBox);

	mainLayout->addItem(valueLayout);

	// Graph
	plot = new QwtPlot(this);
	mainLayout->addWidget(plot);

	// Spacer
	mainLayout->addStretch(10);
}

void SinkView::on_value_changed(const double value)
{
	device_->set_current_limit(value);
}

void SinkView::on_enabled_changed(const bool enabled)
{
	device_->set_enable(enabled);
}

} // namespace views
} // namespace sv
