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

#include <cassert>
#include <tuple>
#include <type_traits>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include <glib.h>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>

#include "configurable.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/properties/baseproperty.hpp"
#include "src/devices/properties/boolproperty.hpp"
#include "src/devices/properties/doubleproperty.hpp"
#include "src/devices/properties/int32property.hpp"
#include "src/devices/properties/measuredquantityproperty.hpp"
#include "src/devices/properties/stringproperty.hpp"
#include "src/devices/properties/uint64property.hpp"

using std::dynamic_pointer_cast;
using std::forward;
using std::make_pair;
using std::make_shared;
using std::string;
using sv::devices::ConfigKey;

namespace sv {
namespace devices {

Configurable::Configurable(
		const shared_ptr<sigrok::Configurable> sr_configurable,
		const QString device_name, const DeviceType device_type):
	sr_configurable_(sr_configurable),
	device_name_(device_name),
	device_type_(device_type)
{
}

/*
template<typename ...Arg>
shared_ptr<Configurable> Configurable::create(Arg&&...arg)
{
	struct make_shared_enabler : public Configurable {
		make_shared_enabler(Arg&&...arg) : Configurable(forward<Arg>(arg)...) {}
	};

	shared_ptr<Configurable> configurable =
		make_shared<make_shared_enabler>(forward<Arg>(arg)...);
	configurable->init();

	return configurable;
}
*/

void Configurable::init()
{
	const auto sr_config_keys = sr_configurable_->config_keys();
	for (auto sr_config_key : sr_config_keys) {
		ConfigKey config_key = deviceutil::get_config_key(sr_config_key);
		if (config_key == ConfigKey::Unknown)
			continue;

		qWarning() << "Configurable::init(): Init " << name() << " - key " <<
			deviceutil::format_config_key(config_key);

		const auto sr_capabilities =
			sr_configurable_->config_capabilities(sr_config_key);
		if (sr_capabilities.count(sigrok::Capability::GET))
			getable_configs_.insert(config_key);
		if (sr_capabilities.count(sigrok::Capability::SET))
			setable_configs_.insert(config_key);
		if (sr_capabilities.count(sigrok::Capability::LIST))
			listable_configs_.insert(config_key);

		shared_ptr<properties::BaseProperty> property;
		const DataType data_type =
			deviceutil::get_data_type_for_config_key(config_key);
		switch (data_type) {
		case devices::DataType::Int32:
			property = make_shared<properties::Int32Property>(
				shared_from_this(), config_key);
			break;
		case devices::DataType::UInt64:
			property = make_shared<properties::UInt64Property>(
				shared_from_this(), config_key);
			break;
		case devices::DataType::Double:
			property = make_shared<properties::DoubleProperty>(
				shared_from_this(), config_key);
			break;
		case devices::DataType::String:
			property = make_shared<properties::StringProperty>(
				shared_from_this(), config_key);
			break;
		case devices::DataType::Bool:
			property = make_shared<properties::BoolProperty>(
				shared_from_this(), config_key);
			break;
		case devices::DataType::MQ:
			property = make_shared<properties::MeasuredQuantityProperty>(
				shared_from_this(), config_key);
			break;
		case devices::DataType::RationalPeriod:
		case devices::DataType::RationalVolt:
		case devices::DataType::Uint64Range:
		case devices::DataType::DoubleRange:
			//qvar = QVariant(g_variant_get_tuple(entry.second.gobj()));
			//break;
		case devices::DataType::KeyValue:
			//qvar = QVariant(g_variant_get_dictionary(entry.second.gobj()));
			//break;
		case devices::DataType::Unknown:
		default:
			assert("Unknown DataType");
		}

		properties_.insert(make_pair(config_key, property));
	}
}

bool Configurable::has_get_config(devices::ConfigKey key)  const
{
	if (getable_configs_.count(key))
		return true;
	return false;
}

template bool Configurable::get_config(devices::ConfigKey) const;
template int32_t Configurable::get_config(devices::ConfigKey) const;
template uint64_t Configurable::get_config(devices::ConfigKey) const;
template double Configurable::get_config(devices::ConfigKey) const;
template std::string Configurable::get_config(devices::ConfigKey) const;
template<typename T> T Configurable::get_config(devices::ConfigKey key) const
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::GET)) {
		qWarning() << "Configurable::get_config(): No key / no getable key " <<
			devices::deviceutil::format_config_key(key);
		assert(false);
	}

	// TODO: implement like get_list
	/*
	try {
	*/
		return Glib::VariantBase::cast_dynamic<Glib::Variant<T>>(
			sr_configurable_->config_get(sr_key)).get();
	/*
	}
	catch (sigrok::Error &error) {
		qWarning() << "Configurable::list_config(): Failed to get key " <<
			devices::deviceutil::format_config_key(key) << ". " << error.what();
		assert(false);
	}

	return ;
	*/
}

Glib::VariantContainerBase Configurable::get_container_config(
	devices::ConfigKey key) const
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::GET)) {
		qWarning() <<
			"Configurable::get_container_config(): No key / no getable key " <<
			devices::deviceutil::format_config_key(key);
		assert(false);
	}

	// TODO: implement like get_list
	/*
	try {
	*/
	Glib::VariantBase gvar = sr_configurable_->config_get(sr_key);
	if (gvar.is_container()) {
		Glib::VariantContainerBase gcontainer =
			Glib::VariantBase::cast_dynamic<Glib::VariantContainerBase>(gvar);
		return gcontainer;
	}
	return Glib::VariantContainerBase();
	/*
	}
	catch (sigrok::Error &error) {
		qWarning() << "Configurable::list_config(): Failed to get key " <<
			devices::deviceutil::format_config_key(key) << ". " << error.what();
		assert(false);
	}

	return ;
	*/
}

bool Configurable::has_set_config(devices::ConfigKey key) const
{
	if (setable_configs_.count(key))
		return true;
	return false;
}

template void Configurable::set_config(devices::ConfigKey, const bool);
template void Configurable::set_config(devices::ConfigKey, const int32_t);
template void Configurable::set_config(devices::ConfigKey, const uint64_t);
template void Configurable::set_config(devices::ConfigKey, const double);
template void Configurable::set_config(devices::ConfigKey, const std::string);
template<typename T> void Configurable::set_config(
	devices::ConfigKey key, const T value)
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::SET)) {
		qWarning() << "Configurable::set_config(): No key / no setable key  " <<
			devices::deviceutil::format_config_key(key);
		assert(false);
	}

	try {
		sr_configurable_->config_set(sr_key, Glib::Variant<T>::create(value));
	}
	catch (sigrok::Error &error) {
		qWarning() << "Configurable::set_config(): Failed to set key " <<
			devices::deviceutil::format_config_key(key) << ". " << error.what();
	}
}

void Configurable::set_container_config(
	devices::ConfigKey key, vector<Glib::VariantBase> childs)
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::SET)) {
		qWarning() <<
			"Configurable::set_container_config(): No key / no setable key  " <<
			devices::deviceutil::format_config_key(key);
		assert(false);
	}

	try {
		sr_configurable_->config_set(
			sr_key, Glib::VariantContainerBase::create_tuple(childs));
	}
	catch (sigrok::Error &error) {
		qWarning() <<
			"Configurable::set_container_config(): Failed to set key " <<
			devices::deviceutil::format_config_key(key) << ". " << error.what();
	}
}

bool Configurable::has_list_config(devices::ConfigKey key) const
{
	if (listable_configs_.count(key))
		return true;
	return false;
}

bool Configurable::list_config(devices::ConfigKey key,
	Glib::VariantContainerBase &gvariant)
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::LIST)) {
		qWarning() <<
			"Configurable::list_config(): No key / no listable key " <<
			devices::deviceutil::format_config_key(key);
		return false;
	}

	try {
		gvariant = sr_configurable_->config_list(sr_key);
	}
	catch (sigrok::Error &error) {
		qWarning() << "Configurable::list_config(): Failed to list key " <<
			devices::deviceutil::format_config_key(key) << ". " << error.what();
		return false;
	}

	return true;
}

QString Configurable::name() const
{
	QString name("?");
	if (auto sr = dynamic_pointer_cast<sigrok::HardwareDevice>(sr_configurable_)) {
		name = device_name_;
	}
	else if (auto sr = dynamic_pointer_cast<sigrok::ChannelGroup>(sr_configurable_)) {
		name = QString("%1 - %2").arg(device_name_).
			arg(QString::fromStdString(sr->name()));
	}
	return name;
}

DeviceType Configurable::device_type() const
{
	return device_type_;
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

map<devices::ConfigKey, shared_ptr<properties::BaseProperty>>
	Configurable::properties() const
{
	return properties_;
}

shared_ptr<properties::BaseProperty>
	Configurable::get_property(devices::ConfigKey config_key) const
{
	if (!properties_.count(config_key))
		return nullptr;
	return properties_.at(config_key);
}

bool Configurable::is_controllable() const
{
	if (setable_configs_.size() > 0 ||
		getable_configs_.size() > 0 ||
		listable_configs_.size() > 0) {

		return true;
	}
	return false;
}

void Configurable::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	for (auto entry : sr_meta->config()) {
		devices::ConfigKey config_key =
			devices::deviceutil::get_config_key(entry.first);

		if (!properties_.count(config_key)) {
			qWarning() << "Configurable::feed_in_meta(): Unknown config key " <<
				QString::fromStdString(entry.first->name()) << " received";
			return;
		}

		properties_[config_key]->on_value_changed(entry.second);

		// TODO: return QVariant from prop->on_value_changed(); and emit
		//Q_EMIT config_changed(config_key, qvar);
	}
}

} // namespace devices
} // namespace sv
