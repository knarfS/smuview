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

#include <QHBoxLayout>
#include <QGroupBox>

#include "measureview.hpp"
#include "src/data/analog.hpp"
#include "src/data/curvedata.hpp"
#include "src/data/signalbase.hpp"
#include "src/widgets/singlevaluepanel.hpp"
#include "src/widgets/plot.hpp"

namespace sv {
namespace views {

MeasureView::MeasureView(shared_ptr<devices::HardwareDevice> device,
		QWidget *parent) :
	QWidget(parent),
	device_(device)
{
	digits_ = 5;
	unit_ = QString("V");

	setup_ui();
	init_values();

	// TODO
	//plot->start();
}


void MeasureView::init_values()
{
}

void MeasureView::setup_ui()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	// Group Box for Setting
	QGroupBox *setValuesGroupBox = new QGroupBox(this);
	setValuesGroupBox->setTitle("Set Values");
	QVBoxLayout *setValuesLayout = new QVBoxLayout(this);
	setValuesLayout->addStretch(10);
	setValuesGroupBox->setLayout(setValuesLayout);

	// Group Box for Values
	QGroupBox *getValuesGroupBox = new QGroupBox(this);
	getValuesGroupBox->setTitle("Actual Values");
	QVBoxLayout *getValuesVLayout = new QVBoxLayout(this);

	// Value panel
	singleValuePanel = new widgets::SingleValuePanel(
		device_->measurement_signal(), this);
	getValuesVLayout->addWidget(singleValuePanel);
	getValuesVLayout->addStretch(5);

	getValuesGroupBox->setLayout(getValuesVLayout);

	QHBoxLayout *valueLayout = new QHBoxLayout(this);
	valueLayout->addWidget(setValuesGroupBox);
	valueLayout->addWidget(getValuesGroupBox);

	mainLayout->addItem(valueLayout);

	// Graph
	/*
	data::CurveData *curve_data = new data::CurveData(
		device_->time_data(),
		device_->measurement_signal()->analog_data());
	plot = new widgets::Plot(curve_data, this);
    plot->setIntervalLength(60);
    plot->setPlotMode(widgets::Plot::PlotModes::Additive);
    plot->setPlotInterval(1);
    //connect( ui->actionConnect, SIGNAL(triggered()), ui->mainPlot, SLOT(start()) );
    //connect( ui->actionDisconnect, SIGNAL(triggered()), ui->mainPlot, SLOT(stop()) );
	mainLayout->addWidget(plot);
	*/

	// Spacer
	mainLayout->addStretch(10);
}

} // namespace views
} // namespace sv

