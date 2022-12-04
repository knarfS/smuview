/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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
#include <set>
#include <utility>
#include <vector>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>
#include <QList>
#include <QSettings>

#include "viewhelper.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/views/dataview.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/democontrolview.hpp"
#include "src/ui/views/genericcontrolview.hpp"
#include "src/ui/views/measurementcontrolview.hpp"
#include "src/ui/views/powerpanelview.hpp"
#include "src/ui/views/scopehorizontalcontrolview.hpp"
#include "src/ui/views/scopetriggercontrolview.hpp"
#include "src/ui/views/scopeverticalcontrolview.hpp"
#include "src/ui/views/sequenceoutputview.hpp"
#include "src/ui/views/smuscriptoutputview.hpp"
#include "src/ui/views/smuscriptview.hpp"
#include "src/ui/views/sourcesinkcontrolview.hpp"
#include "src/ui/views/timeplotview.hpp"
#include "src/ui/views/valuepanelview.hpp"
#include "src/ui/views/xyplotview.hpp"

using std::shared_ptr;
using std::vector;

using sv::devices::ConfigKey;
using sv::devices::DeviceType;

namespace sv {
namespace ui {
namespace views {
namespace viewhelper {

vector<BaseView *> get_views_for_configurable(Session &session,
	shared_ptr<sv::devices::Configurable> configurable)
{
	vector<BaseView *> views;

	if (!configurable)
		return views;

	// Power supplies or electronic loads control view
	if ((configurable->device_type() == DeviceType::PowerSupply ||
		configurable->device_type() == DeviceType::ElectronicLoad) &&
		(configurable->has_get_config(ConfigKey::Enabled) ||
		configurable->has_set_config(ConfigKey::Enabled) ||
		configurable->has_get_config(ConfigKey::Regulation) ||
		configurable->has_set_config(ConfigKey::Regulation) ||
		configurable->has_get_config(ConfigKey::VoltageTarget) ||
		configurable->has_set_config(ConfigKey::VoltageTarget) ||
		configurable->has_get_config(ConfigKey::CurrentLimit) ||
		configurable->has_set_config(ConfigKey::CurrentLimit) ||
		configurable->has_get_config(ConfigKey::OverVoltageProtectionEnabled) ||
		configurable->has_set_config(ConfigKey::OverVoltageProtectionEnabled) ||
		configurable->has_get_config(ConfigKey::OverVoltageProtectionThreshold) ||
		configurable->has_set_config(ConfigKey::OverVoltageProtectionThreshold) ||
		configurable->has_get_config(ConfigKey::OverCurrentProtectionEnabled) ||
		configurable->has_set_config(ConfigKey::OverCurrentProtectionEnabled) ||
		configurable->has_get_config(ConfigKey::OverCurrentProtectionThreshold) ||
		configurable->has_set_config(ConfigKey::OverCurrentProtectionThreshold) ||
		configurable->has_get_config(ConfigKey::UnderVoltageConditionEnabled) ||
		configurable->has_set_config(ConfigKey::UnderVoltageConditionEnabled) ||
		configurable->has_get_config(ConfigKey::UnderVoltageConditionThreshold) ||
		configurable->has_set_config(ConfigKey::UnderVoltageConditionThreshold))) {

		views.push_back(new SourceSinkControlView(session, configurable));
	}

	// Vertical control for scopes
	if (configurable->device_type() == DeviceType::Oscilloscope &&
		(configurable->has_get_config(ConfigKey::Enabled) ||
		configurable->has_set_config(ConfigKey::Enabled) ||
		configurable->has_get_config(ConfigKey::VDiv) ||
		configurable->has_set_config(ConfigKey::VDiv) ||
		configurable->has_get_config(ConfigKey::Coupling) ||
		configurable->has_set_config(ConfigKey::Coupling) ||
		configurable->has_get_config(ConfigKey::Filter) ||
		configurable->has_set_config(ConfigKey::Filter))) {

		views.push_back(new ScopeVerticalControlView(session, configurable));
	}

	// Trigger control for scopes
	if (configurable->device_type() == DeviceType::Oscilloscope &&
		(configurable->has_get_config(ConfigKey::TriggerSource) ||
		configurable->has_set_config(ConfigKey::TriggerSource) ||
		configurable->has_get_config(ConfigKey::TriggerSlope) ||
		configurable->has_set_config(ConfigKey::TriggerSlope) ||
		configurable->has_get_config(ConfigKey::TriggerLevel) ||
		configurable->has_set_config(ConfigKey::TriggerLevel))) {

		views.push_back(new ScopeTriggerControlView(session, configurable));
	}

	// Horizontal control for scopes
	if (configurable->device_type() == DeviceType::Oscilloscope &&
		(configurable->has_get_config(ConfigKey::TimeBase) ||
		configurable->has_set_config(ConfigKey::TimeBase))) {

		views.push_back(new ScopeHorizontalControlView(session, configurable));
	}

	// View for Demo Device
	if (configurable->device_type() == DeviceType::DemoDev &&
		(configurable->has_get_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_set_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_get_config(ConfigKey::Amplitude) ||
		configurable->has_set_config(ConfigKey::Amplitude) ||
		configurable->has_get_config(ConfigKey::Offset) ||
		configurable->has_set_config(ConfigKey::Offset))) {

		views.push_back(new DemoControlView(session, configurable));
	}

	// Measurement devices like DMMs, scales, LCR meters, etc.
	if ((configurable->device_type() == DeviceType::Multimeter ||
		configurable->device_type() == DeviceType::SoundLevelMeter ||
		configurable->device_type() == DeviceType::Thermometer ||
		configurable->device_type() == DeviceType::Hygrometer ||
		configurable->device_type() == DeviceType::Energymeter ||
		configurable->device_type() == DeviceType::LcrMeter ||
		configurable->device_type() == DeviceType::Scale ||
		configurable->device_type() == DeviceType::Powermeter ||
		// TODO: Multiplexers doesn't really fit here
		configurable->device_type() == DeviceType::Multiplexer) &&
		(configurable->has_get_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_set_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_get_config(ConfigKey::Range) ||
		configurable->has_set_config(ConfigKey::Range))) {

		views.push_back(new MeasurementControlView(session, configurable));
	}

	// TODO: SignalGenerators need their own view (waveforms, etc.)

	// Fallback: Generic control view if nothing else fits (e.g. signal
	// generators for now).
	if (views.empty() &&
		(!configurable->getable_configs().empty() ||
		!configurable->setable_configs().empty())) {

		views.push_back(new GenericControlView(session, configurable));
	}

	return views;
}

BaseView *get_view_from_settings(Session &session, QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	QString id = settings.value("id").toString();
	QUuid uuid = settings.value("uuid").toUuid();
	QStringList id_list = id.split(':');
	QString type = id_list[0];

	BaseView *view = nullptr;
	if (type == "data") {
		view = new DataView(session, uuid);
	}
	else if (type == "timeplot") {
		view = new TimePlotView(session, uuid);
	}
	else if (type == "xyplot") {
		view = new XYPlotView(session, uuid);
	}
	else if (type == "powerpanel") {
		view = new PowerPanelView(session, uuid);
	}
	else if (type == "valuepanel") {
		view = new ValuePanelView(session, uuid);
	}
	else if (type == "sequenceoutput") {
		view = new SequenceOutputView(session, uuid);
	}
	else if (type == "smuscriptoutput") {
		view = new SmuScriptOutputView(session, uuid);
	}
	else if (type == "smuscript") {
		view = new SmuScriptView(session, uuid);
	}
	else if (type == "democontrol") {
		view = DemoControlView::init_from_settings(
			session, settings, uuid, origin_device);
	}
	else if (type == "genericcontrol") {
		view = GenericControlView::init_from_settings(
			session, settings, uuid, origin_device);
	}
	else if (type == "measurementcontrol") {
		view = MeasurementControlView::init_from_settings(
			session, settings, uuid, origin_device);
	}
	else if (type == "scopehorizontalcontrol") {
		view = ScopeHorizontalControlView::init_from_settings(
			session, settings, uuid, origin_device);
	}
	else if (type == "scopetriggercontrol") {
		view = ScopeTriggerControlView::init_from_settings(
			session, settings, uuid, origin_device);
	}
	else if (type == "scopeverticalcontrol") {
		view = ScopeVerticalControlView::init_from_settings(
			session, settings, uuid, origin_device);
	}
	else if (type == "sourcesinkcontrol") {
		view = SourceSinkControlView::init_from_settings(
			session, settings, uuid, origin_device);
	}

	if (view)
		view->restore_settings(settings, origin_device);

	return view;
}

} // namespace viewhelper
} // namespace views
} // namespace ui
} // namespace sv
