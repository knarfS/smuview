/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

#include <glib.h>
#include <glibmm.h>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>
#include <QString>

#include "configurable.hpp"
#include "src/data/datautil.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/data/properties/boolproperty.hpp"
#include "src/data/properties/doubleproperty.hpp"
#include "src/data/properties/doublerangeproperty.hpp"
#include "src/data/properties/int32property.hpp"
#include "src/data/properties/measuredquantityproperty.hpp"
#include "src/data/properties/rationalproperty.hpp"
#include "src/data/properties/stringproperty.hpp"
#include "src/data/properties/uint64property.hpp"
#include "src/data/properties/uint64rangeproperty.hpp"
#include "src/devices/configinterface.hpp"
#include "src/devices/configurableref.hpp"
#include "src/devices/deviceref.hpp"

using std::dynamic_pointer_cast;
using std::map;
using std::make_pair;
using std::make_shared;
using std::set;
using std::string;

namespace sv {
namespace devices {

Configurable::Configurable(
		const shared_ptr<sigrok::Configurable> sr_configurable,
		unsigned int configurable_index,
		const devices::DeviceRef &parent_device) :
	name_(init_name(sr_configurable)),
	sr_configurable_(sr_configurable),
	index_(configurable_index),
	parent_device_ref_(parent_device),
	config_interface_(make_shared<ConfigInterface>(sr_configurable))
{
	init_properties();
}

string Configurable::init_name(
	const shared_ptr<sigrok::Configurable> &sr_configurable)
{
	if (auto sr = dynamic_pointer_cast<sigrok::HardwareDevice>(sr_configurable))
		return "";
	if (auto sr = dynamic_pointer_cast<sigrok::ChannelGroup>(sr_configurable))
		return sr->name();
	return "";
}

void Configurable::init_properties()
{
	const ConfigurableRef parent {parent_device_ref_, name_, index_};
	const auto sr_config_keys = sr_configurable_->config_keys();
	for (const auto &sr_config_key : sr_config_keys) {
		ConfigKey config_key = deviceutil::get_config_key(sr_config_key);
		if (config_key == ConfigKey::Unknown)
			continue;

		qWarning() << "Configurable::init(): Init " << display_name()
			<< " - key " << deviceutil::format_config_key(config_key);

		shared_ptr<data::properties::BaseProperty> property;
		const data::DataType data_type =
			deviceutil::get_data_type_for_config_key(config_key);
		switch (data_type) {
		case data::DataType::Int32:
			property = make_shared<data::properties::Int32Property>(
				config_key, config_interface_, parent);
			break;
		case data::DataType::UInt64:
			property = make_shared<data::properties::UInt64Property>(
				config_key, config_interface_, parent);
			break;
		case data::DataType::Double:
			property = make_shared<data::properties::DoubleProperty>(
				config_key, config_interface_, parent);
			break;
		case data::DataType::String:
			property = make_shared<data::properties::StringProperty>(
				config_key, config_interface_, parent);
			break;
		case data::DataType::Bool:
			property = make_shared<data::properties::BoolProperty>(
				config_key, config_interface_, parent);
			break;
		case data::DataType::MQ:
			property = make_shared<data::properties::MeasuredQuantityProperty>(
				config_key, config_interface_, parent);
			break;
		case data::DataType::RationalPeriod:
		case data::DataType::RationalVolt:
			property = make_shared<data::properties::RationalProperty>(
				config_key, config_interface_, parent);
			break;
		case data::DataType::UInt64Range:
			property = make_shared<data::properties::UInt64RangeProperty>(
				config_key, config_interface_, parent);
			break;
		case data::DataType::DoubleRange:
			property = make_shared<data::properties::DoubleRangeProperty>(
				config_key, config_interface_, parent);
			break;
		case data::DataType::KeyValue:
			// TODO: What is KeyValue?
		case data::DataType::Unknown:
		default:
			throw std::runtime_error("Unknown DataType");
		}

		property_map_.insert(make_pair(config_key, property));

		// TODO: This will hold a second source of truth and does not change
		//    after initialized, while property->is_* always queries the devices
		if (property->is_getable())
			getable_configs_.insert(config_key);
		if (property->is_setable())
			setable_configs_.insert(config_key);
		if (property->is_listable())
			listable_configs_.insert(config_key);
	}
}

string Configurable::name() const
{
	return name_;
}

QString Configurable::display_name() const
{
	QString display_name = QString::fromStdString(this->name());
	if (display_name.isEmpty())
		display_name = QString::fromStdString(parent_device_ref_.name);

	return display_name;
}

unsigned int Configurable::index() const
{
	return index_;
}

bool Configurable::has_get_config(devices::ConfigKey config_key) const
{
	return getable_configs_.count(config_key) > 0;
}

bool Configurable::has_set_config(devices::ConfigKey config_key) const
{
	return setable_configs_.count(config_key) > 0;
}

bool Configurable::has_list_config(devices::ConfigKey config_key) const
{
	return listable_configs_.count(config_key) > 0;
}

set<devices::ConfigKey> Configurable::getable_configs() const
{
	return getable_configs_;
}

set<devices::ConfigKey> Configurable::setable_configs() const
{
	return setable_configs_;
}

set<devices::ConfigKey> Configurable::listable_configs() const
{
	return listable_configs_;
}

map<devices::ConfigKey, shared_ptr<data::properties::BaseProperty>>
	Configurable::property_map() const
{
	return property_map_;
}

shared_ptr<data::properties::BaseProperty>
	Configurable::get_property(devices::ConfigKey config_key) const
{
	if (property_map_.count(config_key) == 0)
		return nullptr;
	return property_map_.at(config_key);
}

bool Configurable::is_controllable() const
{
	return !setable_configs_.empty() ||
		!getable_configs_.empty() ||
		!listable_configs_.empty();
}

bool Configurable::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	// TODO: Fix in libsigrok: No list for config! That will make the check if
	//       a config key is existent in this configurable easier!
	for (const auto &entry : sr_meta->config()) {
		devices::ConfigKey config_key =
			devices::deviceutil::get_config_key(entry.first);

		if (property_map_.count(config_key) == 0) {
			qWarning() << "Configurable::feed_in_meta(): Unknown config key "
				<< QString::fromStdString(entry.first->name())
				<< " for configurable " << display_name() << " received";
			// Returning `false` instead of `continue`, because only this way we
			// can somehow handle `sr_meta` for multiple configurables without a
			// configurable reference. See `HardwareDevice::feed_in_meta()`
			return false;
		}

		property_map_[config_key]->on_value_changed(entry.second);

		// TODO: return QVariant from prop->on_value_changed(); and emit
		//Q_EMIT config_changed(config_key, qvar);
	}

	return true;
}

const shared_ptr<devices::ConfigInterface>
&Configurable::config_interface() const
{
	return config_interface_;
}

const devices::DeviceRef &Configurable::parent_device_ref() const
{
	return parent_device_ref_;
}

} // namespace devices
} // namespace sv
