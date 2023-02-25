/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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
#include <QSettings>
#include <QSizePolicy>
#include <QHBoxLayout>
#include <QUuid>
#include <QVBoxLayout>

#include "sourcesinkcontrolview.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/util.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/datatypes/boolbutton.hpp"
#include "src/ui/datatypes/boolled.hpp"
#include "src/ui/datatypes/doublecontrol.hpp"
#include "src/ui/datatypes/stringcombobox.hpp"
#include "src/ui/datatypes/stringled.hpp"
#include "src/ui/datatypes/thresholdcontrol.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/viewhelper.hpp"

using std::shared_ptr;
using sv::devices::ConfigKey;

namespace sv {
namespace ui {
namespace views {

SourceSinkControlView::SourceSinkControlView(Session &session,
		shared_ptr<sv::devices::Configurable> configurable,
		QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	configurable_(configurable)
{
	id_ = "sourcesinkcontrol:" + util::format_uuid(uuid_);

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

	// Regulation selector (sinks) / range selector (power supplies)
	if (configurable_->device_type() == devices::DeviceType::ElectronicLoad) {
		regulation_box_ = new ui::datatypes::StringComboBox(
			configurable_->get_property(ConfigKey::Regulation), true, true);
		info_layout->addWidget(regulation_box_, 1, 0, Qt::AlignLeft);
	}
	else if (configurable_->device_type() == devices::DeviceType::PowerSupply &&
		(configurable_->has_get_config(ConfigKey::Range) ||
		configurable_->has_set_config(ConfigKey::Range) ||
		configurable_->has_list_config(ConfigKey::Range))) {

		range_box_ = new ui::datatypes::StringComboBox(
			configurable_->get_property(ConfigKey::Range), true, true);
		info_layout->addWidget(range_box_, 1, 0, Qt::AlignLeft);
	}

	// Regulation indicators for power supplies
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

	// Protection indicators
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

	// TODO: Change control when switching regulation mode for sinks
	if (configurable_->has_get_config(ConfigKey::VoltageTarget) ||
		configurable_->has_set_config(ConfigKey::VoltageTarget)) {

		voltage_control_ = new ui::datatypes::DoubleControl(
			configurable_->get_property(ConfigKey::VoltageTarget),
			true, true, tr("Voltage"));
		voltage_control_->setSizePolicy(
			QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
		ctrl_layout->addWidget(voltage_control_);
	}

	// TODO: Change control when switching regulation mode for sinks
	if (configurable_->has_get_config(ConfigKey::CurrentLimit) ||
		configurable_->has_set_config(ConfigKey::CurrentLimit)) {

		current_control_ = new ui::datatypes::DoubleControl(
			configurable_->get_property(ConfigKey::CurrentLimit),
			true, true, tr("Current"));
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

void SourceSinkControlView::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	BaseView::save_settings(settings, origin_device);
	SettingsManager::save_configurable(configurable_, settings, origin_device);
}

void SourceSinkControlView::restore_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	BaseView::restore_settings(settings, origin_device);
}

SourceSinkControlView *SourceSinkControlView::init_from_settings(
	Session &session, QSettings &settings, QUuid uuid,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	auto configurable = SettingsManager::restore_configurable(
		session, settings, origin_device);
	if (!configurable)
		return nullptr;
	return new SourceSinkControlView(session, configurable, uuid);
}

} // namespace views
} // namespace ui
} // namespace sv
