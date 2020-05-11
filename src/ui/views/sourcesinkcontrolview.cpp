/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <string>

#include <QSizePolicy>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "sourcesinkcontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/datatypes/boolbutton.hpp"
#include "src/ui/datatypes/boolled.hpp"
#include "src/ui/datatypes/doublecontrol.hpp"
#include "src/ui/datatypes/stringcombobox.hpp"
#include "src/ui/datatypes/stringled.hpp"
#include "src/ui/datatypes/thresholdcontrol.hpp"

using sv::devices::ConfigKey;

namespace sv {
namespace ui {
namespace views {

SourceSinkControlView::SourceSinkControlView(Session &session,
		shared_ptr<sv::devices::Configurable> configurable, QWidget *parent) :
	BaseView(session, parent),
	configurable_(configurable)
{
	id_ = "control:" + std::to_string(BaseView::id_counter++);

	setup_ui();
}

QString SourceSinkControlView::title() const
{
	return tr("Control") + " " + configurable_->display_name();
}

void SourceSinkControlView::setup_ui()
{
	QIcon red_icon(":/icons/status-red.svg");
	QIcon green_icon(":/icons/status-green.svg");
	QIcon grey_icon(":/icons/status-grey.svg");

	QVBoxLayout *layout = new QVBoxLayout();

	QGridLayout *info_layout = new QGridLayout();

	// Enable button
	enable_button_ = new ui::datatypes::BoolButton(
		configurable_->get_property(ConfigKey::Enabled), true, true);
	//info_layout->addWidget(enable_button_, 0, 0, 2, 1,  Qt::AlignLeft);
	info_layout->addWidget(enable_button_, 0, 0, Qt::AlignLeft);

	// Regulation
	if (configurable_->device_type() == devices::DeviceType::ElectronicLoad) {
		regulation_box_ = new ui::datatypes::StringComboBox(
			configurable_->get_property(ConfigKey::Regulation), true, true);
		info_layout->addWidget(regulation_box_, 1, 0, Qt::AlignLeft);
	}
	if (configurable_->device_type() == devices::DeviceType::PowerSupply) {
		cv_led_ = new ui::datatypes::StringLed(
			configurable_->get_property(ConfigKey::Regulation),
			true, green_icon, grey_icon, grey_icon,
			"CV", "", tr("CV"));
		info_layout->addWidget(cv_led_, 0, 1, Qt::AlignLeft);
		cc_led_ = new ui::datatypes::StringLed(
			configurable_->get_property(ConfigKey::Regulation),
			true, red_icon, grey_icon, grey_icon,
			"CC", "", tr("CC"));
		info_layout->addWidget(cc_led_, 1, 1, Qt::AlignLeft);
	}

	ovp_led_ = new ui::datatypes::BoolLed(
		configurable_->get_property(ConfigKey::OverVoltageProtectionActive),
		true, red_icon, grey_icon, grey_icon, tr("OVP"));
	info_layout->addWidget(ovp_led_, 0, 2, Qt::AlignLeft);
	ocp_led_ = new ui::datatypes::BoolLed(
		configurable_->get_property(ConfigKey::OverCurrentProtectionActive),
		true, red_icon, grey_icon, grey_icon, tr("OCP"));
	info_layout->addWidget(ocp_led_, 1, 2, Qt::AlignLeft);
	otp_led_ = new ui::datatypes::BoolLed(
		configurable_->get_property(ConfigKey::OverTemperatureProtectionActive),
		true, red_icon, grey_icon, grey_icon, tr("OTP"));
	info_layout->addWidget(otp_led_, 0, 3, Qt::AlignLeft);
	uvc_led_ = new ui::datatypes::BoolLed(
		configurable_->get_property(ConfigKey::UnderVoltageConditionActive),
		true, red_icon, grey_icon, grey_icon, tr("UVC"));
	info_layout->addWidget(uvc_led_, 1, 3, Qt::AlignLeft);
	info_layout->setColumnStretch(4, 1);
	layout->addLayout(info_layout, 0);

	QHBoxLayout *ctrl_layout = new QHBoxLayout();

	// TODO: Change control for regulation mode (CV, CC, CP, CR) for sinks
	if (configurable_->has_get_config(ConfigKey::VoltageTarget) ||
		configurable_->has_set_config(ConfigKey::VoltageTarget)) {

		voltage_control_ = new ui::datatypes::DoubleControl(
			configurable_->get_property(ConfigKey::VoltageTarget),
			true, true, tr("Voltage"));
		/*
		 * The voltage control and the current control might vary in height,
		 * because the scale numbering of the knobs are probably different.
		 */
		voltage_control_->setSizePolicy(
			QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
		ctrl_layout->addWidget(voltage_control_);
	}

	// TODO: Change control for regulation mode (CV, CC, CP, CR) for sinks
	if (configurable_->has_get_config(ConfigKey::CurrentLimit) ||
		configurable_->has_set_config(ConfigKey::CurrentLimit)) {

		current_control_ = new ui::datatypes::DoubleControl(
			configurable_->get_property(ConfigKey::CurrentLimit),
			true, true, tr("Current"));
		// See above
		current_control_->setSizePolicy(
			QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
		ctrl_layout->addWidget(current_control_, 1, Qt::AlignLeft);
	}

	layout->addLayout(ctrl_layout, 0);

	QHBoxLayout *opt_ctrl_layout = new QHBoxLayout();

	ovp_control_ = new ui::datatypes::ThresholdControl(
		configurable_->get_property(ConfigKey::OverVoltageProtectionThreshold),
		configurable_->get_property(ConfigKey::OverVoltageProtectionEnabled),
		true, true, tr("OVP"));
	opt_ctrl_layout->addWidget(ovp_control_);

	ocp_control_ = new ui::datatypes::ThresholdControl(
		configurable_->get_property(ConfigKey::OverCurrentProtectionThreshold),
		configurable_->get_property(ConfigKey::OverCurrentProtectionEnabled),
		true, true, tr("OCP"));
	opt_ctrl_layout->addWidget(ocp_control_);

	uvc_control_ = new ui::datatypes::ThresholdControl(
		configurable_->get_property(ConfigKey::UnderVoltageConditionThreshold),
		configurable_->get_property(ConfigKey::UnderVoltageConditionEnabled),
		true, true, tr("UVC"));
	opt_ctrl_layout->addWidget(uvc_control_, 1, Qt::AlignLeft);
	layout->addLayout(opt_ctrl_layout, 0);
	layout->addStretch(1);

	this->central_widget_->setLayout(layout);
}

} // namespace views
} // namespace ui
} // namespace sv
