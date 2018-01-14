/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "measurementcontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/hardwaredevice.hpp"

namespace sv {
namespace views {

MeasurementControlView::MeasurementControlView(const Session &session,
		shared_ptr<devices::Configurable> configurable, QWidget *parent) :
	BaseView(session, parent),
	configurable_(configurable)
{
	configurable_->list_measured_quantity(sr_mq_flags_list_, mq_flags_list_);
	setup_ui();
	connect_signals();
	init_values();
}

QString MeasurementControlView::title() const
{
	return configurable_->name() + " " + tr("Control");
}

void MeasurementControlView::setup_ui()
{
	QHBoxLayout *layout = new QHBoxLayout();

	QStringList quantity_list;
	QStringList quantityflags_list;
	if (configurable_->is_measured_quantity_getable()) {
		for (auto pair : mq_flags_list_) {
			quantity_list.append(pair.first);
		}
	}

	quantityBox = new QComboBox();
	quantityBox->addItems(quantity_list);
	layout->addWidget(quantityBox, 0);

	quantityFlagsBox = new QComboBox();
	quantityFlagsBox->addItems(quantityflags_list);
	layout->addWidget(quantityFlagsBox, 0);

	//ctrlLayout->addWidget(regulationBox, 1, Qt::AlignLeft);
	//layout->addLayout(quantityFlagsBox, 0);

	this->centralWidget_->setLayout(layout);
}

void MeasurementControlView::connect_signals()
{
	// Private
	connect(quantityBox, SIGNAL(currentIndexChanged(const QString)),
		this, SLOT(on_quantity_changed(const QString)));
	connect(quantityFlagsBox, SIGNAL(currentIndexChanged(const QString)),
		this, SLOT(on_quantity_flags_changed(const QString)));

}

void MeasurementControlView::init_values()
{
}

void MeasurementControlView::on_quantity_changed(const QString index)
{
	quantityFlagsBox->clear();
	shared_ptr<vector<set<QString>>> flags_vector = mq_flags_list_.at(index);
	for (auto v : *flags_vector) {
		bool first = true;
		QString flags;
		for (QString flag : v) {
			if (!first) flags.append(" ");
			else first = false;

			flags.append(flag);
		}
		quantityFlagsBox->addItem(flags);
	}
}

void MeasurementControlView::on_quantity_flags_changed(/*const QString index*/)
{
	//auto pair = mq_flags_list_.at(index);
}

} // namespace views
} // namespace sv

