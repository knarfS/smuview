/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QVBoxLayout>

#include "measurementcontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/datatypes/measuredquantitycombobox.hpp"

using sv::devices::ConfigKey;

namespace sv {
namespace views {

MeasurementControlView::MeasurementControlView(const Session &session,
		shared_ptr<devices::Configurable> configurable, QWidget *parent) :
	BaseView(session, parent),
	configurable_(configurable)
{
	setup_ui();
}

QString MeasurementControlView::title() const
{
	return configurable_->name() + " " + tr("Control");
}

void MeasurementControlView::setup_ui()
{
	QHBoxLayout *layout = new QHBoxLayout();

	measured_quantity_box_ = new ui::datatypes::MeasuredQuantityComboBox(
		configurable_->get_property(ConfigKey::MeasuredQuantity), true, true);
	layout->addWidget(measured_quantity_box_);

	this->centralWidget_->setLayout(layout);
}

} // namespace views
} // namespace sv
