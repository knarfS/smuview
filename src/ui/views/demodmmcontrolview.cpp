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

#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVariant>

#include "demodmmcontrolview.hpp"
#include "src/session.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/properties/baseproperty.hpp"
#include "src/ui/data/quantitycombobox.hpp"
#include "src/ui/data/quantityflagslist.hpp"
#include "src/ui/datatypes/doublecontrol.hpp"

using sv::devices::ConfigKey;

Q_DECLARE_METATYPE(sv::data::measured_quantity_t)

namespace sv {
namespace ui {
namespace views {

DemoDMMControlView::DemoDMMControlView(const Session &session,
		shared_ptr<sv::devices::Configurable> configurable, QWidget *parent) :
	BaseView(session, parent),
	configurable_(configurable)
{
	setup_ui();
	connect_signals();
}

QString DemoDMMControlView::title() const
{
	return configurable_->name() + " " + tr("Control");
}

void DemoDMMControlView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	// The demo dmm device has no listable measurement quantities /
	// qunatity flags, so we use all...
	quantity_box_ = new ui::data::QuantityComboBox();
	layout->addWidget(quantity_box_);
	quantity_flags_list_ = new ui::data::QuantityFlagsList();
	layout->addWidget(quantity_flags_list_);

	set_button_ = new QPushButton();
	set_button_->setText(tr("Set"));
	layout->addWidget(set_button_, 0);

	QHBoxLayout *controls_layout = new QHBoxLayout();

	amplitude_control_ = new ui::datatypes::DoubleControl(
		configurable_->get_property(ConfigKey::Amplitude),
		true, true, tr("Amplitude"));
	controls_layout->addWidget(amplitude_control_);

	/*
	offset_control_ = new ui::datatypes::DoubleControl(
		configurable_->get_property(ConfigKey::Offset),
		true, true, tr("Offset"));
	controls_layout->addWidget(offset_control_);
	*/

	layout->addLayout(controls_layout);

	this->central_widget_->setLayout(layout);
}

void DemoDMMControlView::connect_signals()
{
	// Control elements -> Device
	connect(set_button_, SIGNAL(clicked(bool)), this, SLOT(on_quantity_set()));

	// Device -> control elements
}

void DemoDMMControlView::on_quantity_set()
{
	sv::data::Quantity quantity = quantity_box_->selected_quantity();
	set<sv::data::QuantityFlag> quantity_flags =
		quantity_flags_list_->selected_quantity_flags();

	auto mq = make_pair(quantity, quantity_flags);
	auto prop = configurable_->get_property(ConfigKey::MeasuredQuantity);
	prop->change_value(QVariant().fromValue(mq));
}

} // namespace views
} // namespace ui
} // namespace sv
