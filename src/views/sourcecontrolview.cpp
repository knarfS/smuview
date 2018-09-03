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

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "sourcecontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/widgets/controlbutton.hpp"
#include "src/widgets/led.hpp"
#include "src/widgets/optionalvaluecontrol.hpp"
#include "src/widgets/valuecontrol.hpp"

using sv::devices::ConfigKey;

namespace sv {
namespace views {

SourceControlView::SourceControlView(const Session &session,
	shared_ptr<devices::Configurable> configurable, QWidget *parent) :
		BaseView(session, parent),
	configurable_(configurable)
{
	setup_ui();
	connect_signals();
	init_values();
}

QString SourceControlView::title() const
{
	return configurable_->name() + " " + tr("Control");
}

void SourceControlView::setup_ui()
{
	double min;
	double max;
	double step;
	QIcon red_icon(":/icons/status-red.svg");
	QIcon green_icon(":/icons/status-green.svg");
	QIcon grey_icon(":/icons/status-grey.svg");

	QVBoxLayout *layout = new QVBoxLayout();

	QGridLayout *infoLayout = new QGridLayout();

	// Enable button
	enableButton = new widgets::ControlButton(
		configurable_->has_get_config(ConfigKey::Enabled),
		configurable_->has_set_config(ConfigKey::Enabled));
	infoLayout->addWidget(enableButton, 0, 0, 2, 1,  Qt::AlignLeft);

	// TODO
	// CV
	//infoLayout->addWidget(ovpLed, 0, 1, Qt::AlignLeft);
	// CC
	//infoLayout->addWidget(ovpLed, 1, 1, Qt::AlignLeft);

	ovpLed = new widgets::Led(
		configurable_->has_get_config(ConfigKey::OverVoltageProtectionActive),
		tr("OVP"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(ovpLed, 0, 2, Qt::AlignLeft);
	ocpLed = new widgets::Led(
		configurable_->has_get_config(ConfigKey::OverCurrentProtectionActive),
		tr("OCP"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(ocpLed, 1, 2, Qt::AlignLeft);
	otpLed = new widgets::Led(
		configurable_->has_get_config(ConfigKey::OverTemperatureProtectionActive),
		tr("OTP"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(otpLed, 0, 3, Qt::AlignLeft);
	uvcLed = new widgets::Led(
		configurable_->has_get_config(ConfigKey::UnderVoltageConditionActive),
		tr("UVC"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(uvcLed, 1, 3, Qt::AlignLeft);
	layout->addLayout(infoLayout, 0);

	QHBoxLayout *ctrlLayout = new QHBoxLayout();

	min = max = step = 0;
	configurable_->list_config_min_max_steps(
		ConfigKey::VoltageTarget, min, max, step);
	setVoltageControl = new widgets::ValueControl(
		tr("Voltage"), 3, tr("V"), min, max, step);
	ctrlLayout->addWidget(setVoltageControl);

	min = max = step = 0;
	configurable_->list_config_min_max_steps(
		ConfigKey::CurrentLimit, min, max, step);
	setCurrentControl = new widgets::ValueControl(
		tr("Current"), 3, tr("A"), min, max, step);
	ctrlLayout->addWidget(setCurrentControl, 1, Qt::AlignLeft);
	layout->addLayout(ctrlLayout, 0);

	QHBoxLayout *optCtrlLayout = new QHBoxLayout();

	min = max = step = 0;
	configurable_->list_config_min_max_steps(
		ConfigKey::OverVoltageProtectionThreshold, min, max, step);
	ovpControl = new widgets::OptionalValueControl(
		configurable_->has_get_config(ConfigKey::OverVoltageProtectionEnabled),
		configurable_->has_set_config(ConfigKey::OverVoltageProtectionEnabled),
		configurable_->has_get_config(ConfigKey::OverVoltageProtectionThreshold),
		configurable_->has_set_config(ConfigKey::OverVoltageProtectionThreshold),
		tr("OVP"), tr("V"), min, max, step);
	optCtrlLayout->addWidget(ovpControl);

	min = max = step = 0;
	configurable_->list_config_min_max_steps(
		ConfigKey::OverCurrentProtectionThreshold, min, max, step);
	ocpControl = new widgets::OptionalValueControl(
		configurable_->has_get_config(ConfigKey::OverCurrentProtectionEnabled),
		configurable_->has_set_config(ConfigKey::OverCurrentProtectionEnabled),
		configurable_->has_get_config(ConfigKey::OverCurrentProtectionThreshold),
		configurable_->has_set_config(ConfigKey::OverCurrentProtectionThreshold),
		tr("OCP"), tr("A"), min, max, step);
	optCtrlLayout->addWidget(ocpControl);

	min = max = step = 0;
	configurable_->list_config_min_max_steps(
		ConfigKey::UnderVoltageConditionThreshold, min, max, step);
	uvcControl = new widgets::OptionalValueControl(
		configurable_->has_get_config(ConfigKey::UnderVoltageConditionEnabled),
		configurable_->has_set_config(ConfigKey::UnderVoltageConditionEnabled),
		configurable_->has_get_config(ConfigKey::UnderVoltageConditionThreshold),
		configurable_->has_set_config(ConfigKey::UnderVoltageConditionThreshold),
		tr("UVC"), tr("V"), min, max, step);
	optCtrlLayout->addWidget(uvcControl, 1, Qt::AlignLeft);
	layout->addLayout(optCtrlLayout, 0);
	layout->addStretch(1);

	this->centralWidget_->setLayout(layout);
}

void SourceControlView::connect_signals()
{
	// Control elements -> Device
	connect(enableButton, SIGNAL(state_changed(const bool)),
		this, SLOT(on_enabled_changed(const bool)));
	connect(setVoltageControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_voltage_changed(const double)));
	connect(setCurrentControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_current_changed(const double)));
	connect(ovpControl, SIGNAL(state_changed(const bool)),
		this, SLOT(on_ovp_enabled_changed(const bool)));
	connect(ovpControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_ovp_threshold_changed(const double)));
	connect(ocpControl, SIGNAL(state_changed(const bool)),
		this, SLOT(on_ocp_enabled_changed(const bool)));
	connect(ocpControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_ocp_threshold_changed(const double)));
	connect(uvcControl, SIGNAL(state_changed(const bool)),
		this, SLOT(on_uvc_enabled_changed(const bool)));
	connect(uvcControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_uvc_threshold_changed(const double)));;

	// Device -> Control elements
	connect(configurable_.get(), SIGNAL(enabled_changed(const bool)),
		enableButton, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(voltage_target_changed(const double)),
		setVoltageControl, SLOT(change_value(const double)));
	connect(configurable_.get(), SIGNAL(current_limit_changed(const double)),
		setCurrentControl, SLOT(change_value(const double)));
	connect(configurable_.get(), SIGNAL(ovp_enabled_changed(const bool)),
		ovpControl, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(ovp_threshold_changed(const double)),
		ovpControl, SLOT(change_value(const double)));
	connect(configurable_.get(), SIGNAL(ocp_enabled_changed(const bool)),
		ocpControl, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(ocp_threshold_changed(const double)),
		ocpControl, SLOT(change_value(const double)));
	connect(configurable_.get(), SIGNAL(uvc_enabled_changed(const bool)),
		uvcControl, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(uvc_threshold_changed(const double)),
		uvcControl, SLOT(change_value(const double)));

	// Device -> LEDs
	connect(configurable_.get(), SIGNAL(ovp_active_changed(const bool)),
		ovpLed, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(ocp_active_changed(const bool)),
		ocpLed, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(uvc_active_changed(const bool)),
		uvcLed, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(otp_active_changed(const bool)),
		otpLed, SLOT(change_state(const bool)));
}

void SourceControlView::init_values()
{
	if (configurable_->has_get_config(ConfigKey::Enabled))
		enableButton->change_state(
			configurable_->get_config<bool>(ConfigKey::Enabled));
	if (configurable_->has_get_config(ConfigKey::VoltageTarget))
		setVoltageControl->change_value(
			configurable_->get_config<double>(ConfigKey::VoltageTarget));
	if (configurable_->has_get_config(ConfigKey::CurrentLimit))
		setCurrentControl->change_value(
			configurable_->get_config<double>(ConfigKey::CurrentLimit));
	if (configurable_->has_get_config(ConfigKey::OverVoltageProtectionEnabled))
		ovpControl->change_state(
			configurable_->get_config<bool>(ConfigKey::OverVoltageProtectionEnabled));
	if (configurable_->has_get_config(ConfigKey::OverVoltageProtectionThreshold))
		ovpControl->change_value(
			configurable_->get_config<double>(ConfigKey::OverVoltageProtectionThreshold));
	if (configurable_->has_get_config(ConfigKey::OverCurrentProtectionEnabled))
		ocpControl->change_state(
			configurable_->get_config<bool>(ConfigKey::OverCurrentProtectionEnabled));
	if (configurable_->has_get_config(ConfigKey::OverCurrentProtectionThreshold))
		ocpControl->change_value(
			configurable_->get_config<double>(ConfigKey::OverCurrentProtectionThreshold));
	if (configurable_->has_get_config(ConfigKey::UnderVoltageConditionEnabled))
		uvcControl->change_state(
			configurable_->get_config<bool>(ConfigKey::UnderVoltageConditionEnabled));
	if (configurable_->has_get_config(ConfigKey::UnderVoltageConditionThreshold))
		uvcControl->change_value(
			configurable_->get_config<double>(ConfigKey::UnderVoltageConditionThreshold));

	// LEDs
	if (configurable_->has_get_config(ConfigKey::OverVoltageProtectionActive))
		ovpLed->change_state(
			configurable_->get_config<bool>(ConfigKey::OverVoltageProtectionActive));
	if (configurable_->has_get_config(ConfigKey::OverCurrentProtectionActive))
		ocpLed->change_state(
			configurable_->get_config<bool>(ConfigKey::OverCurrentProtectionActive));
	if (configurable_->has_get_config(ConfigKey::OverTemperatureProtectionActive))
		otpLed->change_state(
			configurable_->get_config<bool>(ConfigKey::OverTemperatureProtectionActive));
	if (configurable_->has_get_config(ConfigKey::UnderVoltageConditionActive))
		uvcLed->change_state(
			configurable_->get_config<bool>(ConfigKey::UnderVoltageConditionActive));
}

void SourceControlView::on_enabled_changed(const bool enabled)
{
	configurable_->set_config(ConfigKey::Enabled, enabled);
}

void SourceControlView::on_voltage_changed(const double value)
{
	configurable_->set_config(ConfigKey::VoltageTarget, value);
}

void SourceControlView::on_current_changed(const double value)
{
	configurable_->set_config(ConfigKey::CurrentLimit, value);
}

void SourceControlView::on_ovp_enabled_changed(const bool enabled)
{
	configurable_->set_config(ConfigKey::OverVoltageProtectionEnabled, enabled);
}

void SourceControlView::on_ovp_threshold_changed(const double value)
{
	configurable_->set_config(ConfigKey::OverVoltageProtectionThreshold, value);
}

void SourceControlView::on_ocp_enabled_changed(const bool enabled)
{
	configurable_->set_config(ConfigKey::OverCurrentProtectionEnabled, enabled);
}

void SourceControlView::on_ocp_threshold_changed(const double value)
{
	configurable_->set_config(ConfigKey::OverCurrentProtectionThreshold, value);
}

void SourceControlView::on_uvc_enabled_changed(const bool enabled)
{
	configurable_->set_config(ConfigKey::UnderVoltageConditionEnabled, enabled);
}

void SourceControlView::on_uvc_threshold_changed(const double value)
{
	configurable_->set_config(ConfigKey::UnderVoltageConditionThreshold, value);
}

} // namespace views
} // namespace sv

