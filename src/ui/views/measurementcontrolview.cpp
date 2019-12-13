/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <string>

#include <QDebug>
#include <QFormLayout>

#include "measurementcontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/datatypes/measuredquantitycombobox.hpp"
#include "src/ui/datatypes/stringcombobox.hpp"

using sv::devices::ConfigKey;

namespace sv {
namespace ui {
namespace views {

MeasurementControlView::MeasurementControlView(Session &session,
		shared_ptr<sv::devices::Configurable> configurable, QWidget *parent) :
	BaseView(session, parent),
	configurable_(configurable)
{
	id_ = "control:" + configurable_->name();

	setup_ui();
}

QString MeasurementControlView::title() const
{
	return tr("Control") + " " + configurable_->display_name();
}

void MeasurementControlView::setup_ui()
{
	QFormLayout *layout = new QFormLayout();

	measured_quantity_box_ = new ui::datatypes::MeasuredQuantityComboBox(
		configurable_->get_property(ConfigKey::MeasuredQuantity), true, true);
	layout->addRow(tr("Quantity"), measured_quantity_box_);

	range_box_ = new ui::datatypes::StringComboBox(
		configurable_->get_property(ConfigKey::Range), true, true);
	layout->addRow(tr("Range"), range_box_);

	digits_box_ = new ui::datatypes::StringComboBox(
		configurable_->get_property(ConfigKey::Digits), true, true);
	layout->addRow(tr("Digits"), digits_box_);

	this->central_widget_->setLayout(layout);
}

} // namespace views
} // namespace ui
} // namespace sv
