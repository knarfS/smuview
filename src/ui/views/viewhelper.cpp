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
#include <set>
#include <utility>
#include <vector>

#include <QDebug>
#include <QList>
#include <QSettings>

#include "viewhelper.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/views/dataview.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/democontrolview.hpp"
#include "src/ui/views/genericcontrolview.hpp"
#include "src/ui/views/measurementcontrolview.hpp"
#include "src/ui/views/plotview.hpp"
#include "src/ui/views/powerpanelview.hpp"
#include "src/ui/views/sourcesinkcontrolview.hpp"
#include "src/ui/views/valuepanelview.hpp"

using std::dynamic_pointer_cast;
using std::make_pair;
using std::shared_ptr;
using std::vector;

using sv::devices::ConfigKey;
using sv::devices::DeviceType;

namespace sv {
namespace ui {
namespace views {
namespace viewhelper {
namespace {

shared_ptr<sv::devices::BaseDevice> get_device(Session &session,
	QSettings &settings, QString key_prefix = "")
{
	QString device_key = key_prefix + "device";

	if (!settings.contains(device_key))
		return nullptr;

	string device_id = settings.value(device_key).toString().toStdString();
	if (session.devices().count(device_id) == 0) // TODO: Rename devices() to device_map()
		return nullptr;

	return session.devices()[device_id];
}

shared_ptr<sv::devices::Configurable> get_configurable(Session &session,
	QSettings &settings, QString key_prefix = "")
{
	QString configurable_key = key_prefix + "configurable";

	auto device = get_device(session, settings, key_prefix);
	if (!device)
		return nullptr;

	if (!settings.contains(configurable_key))
		return nullptr;

	string conf_id = settings.value(configurable_key).toString().toStdString();
	if (device->configurable_map().count(conf_id) == 0)
		return nullptr;

	return device->configurable_map()[conf_id];
}

shared_ptr<sv::channels::BaseChannel> get_channel(Session &session,
	QSettings &settings, QString key_prefix = "")
{
	QString channel_key = key_prefix + "channel";

	auto device = get_device(session, settings, key_prefix);
	if (!device)
		return nullptr;

	if (!settings.contains(channel_key))
		return nullptr;

	string channel_id = settings.value(channel_key).toString().toStdString();
	if (device->channel_map().count(channel_id) == 0)
		return nullptr;

	return device->channel_map()[channel_id];
}

/*
shared_ptr<sv::channels::BaseChannel> get_channel_from_group(Session &session,
	QSettings &settings, QString group, QString key_prefix = "")
{
	settings.beginGroup(group);
	auto channel = get_channel(session, settings, key_prefix);
	settings.endGroup();

	return channel;
}
*/

shared_ptr<sv::data::AnalogTimeSignal> get_signal(Session &session,
	QSettings &settings, QString key_prefix = "")
{
	QString signal_key = key_prefix + "signal";

	auto channel = get_channel(session, settings, key_prefix);
	if (!channel)
		return nullptr;

	if (!settings.contains(signal_key+"_sr_q") ||
			!settings.contains(signal_key+"_sr_qf"))
		return nullptr;

	auto sr_q = settings.value(signal_key+"_sr_q").value<uint32_t>();
	auto sr_qf = settings.value(signal_key+"_sr_qf").value<uint64_t>();
	auto mq = make_pair(
		sv::data::datautil::get_quantity(sr_q),
		sv::data::datautil::get_quantity_flags(sr_qf));
	if (channel->signal_map().count(mq) == 0)
		return nullptr;

	auto signal = dynamic_pointer_cast<sv::data::AnalogTimeSignal>(
		channel->signal_map()[mq][0]);
	if (!signal)
		return nullptr;

	return signal;
}

shared_ptr<sv::data::AnalogTimeSignal> get_signal_from_group(Session &session,
	QSettings &settings, QString group, QString key_prefix = "")
{
	settings.beginGroup(group);
	auto signal = get_signal(session, settings, key_prefix);
	settings.endGroup();

	return signal;
}

} // namespace

BaseView *get_view_for_configurable(Session &session,
	shared_ptr<sv::devices::Configurable> configurable)
{
	if (!configurable)
		return nullptr;

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

		return new SourceSinkControlView(session, configurable);
	}

	// View for Demo Device
	if (configurable->device_type() == DeviceType::DemoDev &&
		(configurable->has_get_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_set_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_get_config(ConfigKey::Amplitude) ||
		configurable->has_set_config(ConfigKey::Amplitude) ||
		configurable->has_get_config(ConfigKey::Offset) ||
		configurable->has_set_config(ConfigKey::Offset))) {

		return new DemoControlView(session, configurable);
	}

	// Measurement devices like DMMs, scales, LCR meters, etc.
	if ((configurable->device_type() == DeviceType::Multimeter ||
		configurable->device_type() == DeviceType::SoundLevelMeter ||
		configurable->device_type() == DeviceType::Thermometer ||
		configurable->device_type() == DeviceType::Hygrometer ||
		configurable->device_type() == DeviceType::Energymeter ||
		configurable->device_type() == DeviceType::LcrMeter ||
		configurable->device_type() == DeviceType::Scale ||
		configurable->device_type() == DeviceType::Powermeter) &&
		(configurable->has_get_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_set_config(ConfigKey::MeasuredQuantity) ||
		configurable->has_get_config(ConfigKey::Range) ||
		configurable->has_set_config(ConfigKey::Range))) {

		return new MeasurementControlView(session, configurable);
	}

	// Generic control view
	if (!configurable->getable_configs().empty() ||
		!configurable->setable_configs().empty()) {

		return new GenericControlView(session, configurable);
	}

	return nullptr;
}

bool has_device_settings(shared_ptr<sv::devices::BaseDevice> device)
{
	QSettings settings;
	if (settings.childGroups().contains(QString::fromStdString(device->id()))) {
		qWarning() << "has_device_settings(): Settings for " <<
			QString::fromStdString(device->id()) << " found!";
		return true;
	}
	qWarning() << "has_device_settings(): NO settings for " <<
		QString::fromStdString(device->id()) << " found!";
	return false;
}

BaseView *get_view_from_settings(Session &session, QSettings &settings)
{
	qWarning() << "get_view_from_settings(): current group = " << settings.group();

	QString id = settings.value("id").toString();
	QUuid uuid = settings.value("uuid").toUuid();
	QStringList id_list = id.split(':');
	QString type = id_list[0];
	qWarning() << "get_view_from_settings(): type = " << type;

	if (type == "data") {
		vector<shared_ptr<sv::data::AnalogTimeSignal>> signals;
		for (const auto &group : settings.childGroups()) {
			if (group.startsWith("signal")) {
				signals.push_back(
					get_signal_from_group(session, settings, group));
			}
		}
		DataView *view = nullptr;
		if (!signals.empty()) {
			view = new DataView(session, signals.at(0), uuid);
			for (size_t i=1; i<signals.size(); i++)
				view->add_signal(signals.at(i));
		}
		return view;
	}
	if (type == "plot_ch") {
		vector<shared_ptr<sv::data::AnalogTimeSignal>> signals;
		for (const auto &group : settings.childGroups()) {
			if (group.startsWith("curve")) {
				signals.push_back(
					get_signal_from_group(session, settings, group));
			}
		}
		PlotView *view = nullptr;
		if (!signals.empty()) {
			view = new PlotView(session, signals.at(0)->parent_channel(), uuid);
			for (size_t i=1; i<signals.size(); i++)
				view->add_time_curve(signals.at(i));
		}
		return view;
	}
	if (type == "plot_sig") {
		vector<shared_ptr<sv::data::AnalogTimeSignal>> signals;
		for (const auto &group : settings.childGroups()) {
			if (group.startsWith("curve")) {
				signals.push_back(
					get_signal_from_group(session, settings, group));
			}
		}
		PlotView *view = nullptr;
		if (!signals.empty()) {
			view = new PlotView(session, signals.at(0), uuid);
			for (size_t i=1; i<signals.size(); i++)
				view->add_time_curve(signals.at(i));
		}
		return view;
	}
	if (type == "plot_xy") {
		/*
		auto x_signal = get_signal(session, settings, "x_");
		if (!x_signal)
			return nullptr;
		auto y_signal = get_signal(session, settings, "y_");
		if (!y_signal)
			return nullptr;
		return new PlotView(session, x_signal, y_signal, uuid);
		*/
	}
	if (type == "powerpanel") {
		auto v_signal = get_signal(session, settings, "v_");
		if (!v_signal)
			return nullptr;
		auto i_signal = get_signal(session, settings, "i_");
		if (!i_signal)
			return nullptr;
		return new PowerPanelView(session, v_signal, i_signal, uuid);
	}
	if (type == "valuepanel_ch") {
		auto channel = get_channel(session, settings);
		if (!channel)
			return nullptr;
		return new ValuePanelView(session, channel, uuid);
	}
	if (type == "valuepanel_sig") {
		auto signal = get_signal(session, settings);
		if (!signal)
			return nullptr;
		return new ValuePanelView(session, signal, uuid);
	}
	if (type == "sequenceoutput") {
		// TODO
	}
	if (type == "democontrol") {
		auto configurable = get_configurable(session, settings);
		if (!configurable)
			return nullptr;
		return new DemoControlView(session, configurable, uuid);
	}
	if (type == "genericcontrol") {
		auto configurable = get_configurable(session, settings);
		if (!configurable)
			return nullptr;
		return new GenericControlView(session, configurable, uuid);
	}
	if (type == "measurementcontrol") {
		auto configurable = get_configurable(session, settings);
		if (!configurable)
			return nullptr;
		return new MeasurementControlView(session, configurable, uuid);
	}
	if (type == "sourcesinkcontrol") {
		auto configurable = get_configurable(session, settings);
		if (!configurable)
			return nullptr;
		return new SourceSinkControlView(session, configurable, uuid);
	}

	return nullptr;

	/*
	"sequence:" + property_->name(); // SequenceOutputView

	"smuscriptoutput:"; // SmuScriptOutputView
	"smuscript:"; // SmuScriptView
	*/
}

} // namespace viewhelper
} // namespace views
} // namespace ui
} // namespace sv
