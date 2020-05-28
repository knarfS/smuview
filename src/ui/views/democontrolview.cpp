/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>
#include <string>

#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QUuid>
#include <QVBoxLayout>
#include <QVariant>

#include "democontrolview.hpp"
#include "src/session.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/measuredquantityproperty.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/data/quantitycombobox.hpp"
#include "src/ui/data/quantityflagslist.hpp"
#include "src/ui/datatypes/doublecontrol.hpp"
#include "src/ui/datatypes/stringcombobox.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/viewhelper.hpp"

using std::static_pointer_cast;
using sv::devices::ConfigKey;

Q_DECLARE_METATYPE(sv::data::measured_quantity_t)

namespace sv {
namespace ui {
namespace views {

DemoControlView::DemoControlView(Session &session,
		shared_ptr<sv::devices::Configurable> configurable,
		QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	configurable_(configurable)
{
	id_ = "democontrol:" + uuid_.toString(QUuid::WithoutBraces).toStdString();

	setup_ui();
	connect_signals();
}

QString DemoControlView::title() const
{
	return tr("Control") + " " + configurable_->display_name();
}

void DemoControlView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	if (configurable_->has_get_config(ConfigKey::MeasuredQuantity) ||
		configurable_->has_set_config(ConfigKey::MeasuredQuantity)) {

		// The demo dmm device has no listable measurement quantities /
		// quantity flags, so we use all...
		auto mq_prop =
			static_pointer_cast<sv::data::properties::MeasuredQuantityProperty>(
				configurable_->get_property(ConfigKey::MeasuredQuantity));

		quantity_box_ = new ui::data::QuantityComboBox();
		quantity_box_->select_quantity(
			mq_prop->measured_quantity_value().first);
		layout->addWidget(quantity_box_);
		quantity_flags_list_ = new ui::data::QuantityFlagsList();
		quantity_flags_list_->select_quantity_flags(
			mq_prop->measured_quantity_value().second);
		layout->addWidget(quantity_flags_list_);

		set_button_ = new QPushButton();
		set_button_->setText(tr("Set"));
		layout->addWidget(set_button_, 0);
	}

	if (configurable_->has_get_config(ConfigKey::PatternMode) ||
		configurable_->has_set_config(ConfigKey::PatternMode)) {

		pattern_box_ = new ui::datatypes::StringComboBox(
			configurable_->get_property(ConfigKey::PatternMode), true, true);
		layout->addWidget(pattern_box_);
	}

	QHBoxLayout *controls_layout = new QHBoxLayout();

	amplitude_control_ = new ui::datatypes::DoubleControl(
		configurable_->get_property(ConfigKey::Amplitude),
		true, true, tr("Amplitude"));
	controls_layout->addWidget(amplitude_control_);

	offset_control_ = new ui::datatypes::DoubleControl(
		configurable_->get_property(ConfigKey::Offset),
		true, true, tr("Offset"));
	controls_layout->addWidget(offset_control_);

	layout->addLayout(controls_layout);

	this->central_widget_->setLayout(layout);
}

void DemoControlView::connect_signals()
{
	// Control elements -> Device
	if (configurable_->has_get_config(ConfigKey::MeasuredQuantity) ||
		configurable_->has_set_config(ConfigKey::MeasuredQuantity)) {

		connect(set_button_, SIGNAL(clicked(bool)),
			this, SLOT(on_quantity_set()));
	}

	// Device -> control elements
}

void DemoControlView::save_settings(QSettings &settings) const
{
	BaseView::save_settings(settings);
	viewhelper::save_configurable(configurable_, settings);
}

void DemoControlView::restore_settings(QSettings &settings)
{
	BaseView::restore_settings(settings);
}

void DemoControlView::on_quantity_set()
{
	sv::data::Quantity quantity = quantity_box_->selected_quantity();
	set<sv::data::QuantityFlag> quantity_flags =
		quantity_flags_list_->selected_quantity_flags();

	auto mq = make_pair(quantity, quantity_flags);
	auto prop = configurable_->get_property(ConfigKey::MeasuredQuantity);
	prop->change_value(QVariant::fromValue(mq));
}

} // namespace views
} // namespace ui
} // namespace sv
