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

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "sourcecontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/datatypes/boolbutton.hpp"
#include "src/ui/datatypes/boolled.hpp"
#include "src/ui/datatypes/doublecontrol.hpp"
#include "src/ui/datatypes/thresholdcontrol.hpp"

using sv::devices::ConfigKey;

namespace sv {
namespace ui {
namespace views {

SourceControlView::SourceControlView(const Session &session,
		shared_ptr<sv::devices::Configurable> configurable, QWidget *parent) :
	BaseView(session, parent),
	configurable_(configurable)
{
	setup_ui();
}

QString SourceControlView::title() const
{
	return configurable_->name() + " " + tr("Control");
}

void SourceControlView::setup_ui()
{
	QIcon red_icon(":/icons/status-red.svg");
	QIcon green_icon(":/icons/status-green.svg");
	QIcon grey_icon(":/icons/status-grey.svg");

	QVBoxLayout *layout = new QVBoxLayout();

	QGridLayout *infoLayout = new QGridLayout();

	// Enable button
	enable_button_ = new ui::datatypes::BoolButton(
		configurable_->get_property(ConfigKey::Enabled), true, true);
	infoLayout->addWidget(enable_button_, 0, 0, 2, 1,  Qt::AlignLeft);

	// TODO
	// CV
	//infoLayout->addWidget(cv_led_, 0, 1, Qt::AlignLeft);
	// CC
	//infoLayout->addWidget(cc_led_, 1, 1, Qt::AlignLeft);

	ovp_led_ = new ui::datatypes::BoolLed(
		configurable_->get_property(ConfigKey::OverVoltageProtectionActive),
		true, red_icon, grey_icon, grey_icon, tr("OVP"));
	infoLayout->addWidget(ovp_led_, 0, 2, Qt::AlignLeft);
	ocp_led_ = new ui::datatypes::BoolLed(
		configurable_->get_property(ConfigKey::OverCurrentProtectionActive),
		true, red_icon, grey_icon, grey_icon, tr("OCP"));
	infoLayout->addWidget(ocp_led_, 1, 2, Qt::AlignLeft);
	otp_led_ = new ui::datatypes::BoolLed(
		configurable_->get_property(ConfigKey::OverTemperatureProtectionActive),
		true, red_icon, grey_icon, grey_icon, tr("OTP"));
	infoLayout->addWidget(otp_led_, 0, 3, Qt::AlignLeft);
	uvc_led_ = new ui::datatypes::BoolLed(
		configurable_->get_property(ConfigKey::UnderVoltageConditionActive),
		true, red_icon, grey_icon, grey_icon, tr("UVC"));
	infoLayout->addWidget(uvc_led_, 1, 3, Qt::AlignLeft);
	layout->addLayout(infoLayout, 0);

	QHBoxLayout *ctrl_layout = new QHBoxLayout();

	voltage_control_ = new ui::datatypes::DoubleControl(
		configurable_->get_property(ConfigKey::VoltageTarget),
		true, true, tr("Voltage"));
	ctrl_layout->addWidget(voltage_control_);

	current_control_ = new ui::datatypes::DoubleControl(
		configurable_->get_property(ConfigKey::CurrentLimit),
		true, true, tr("Current"));
	ctrl_layout->addWidget(current_control_, 1, Qt::AlignLeft);
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

	this->centralWidget_->setLayout(layout);
}

} // namespace views
} // namespace ui
} // namespace sv
