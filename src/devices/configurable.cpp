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

#include <cassert>
#include <tuple>
#include <type_traits>
#include <map>
#include <memory>
#include <string>
#include <utility>

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

using std::dynamic_pointer_cast;
using std::make_pair;
using std::make_shared;
using std::string;
using sv::devices::ConfigKey;

namespace sv {
namespace devices {

Configurable::Configurable(
		const shared_ptr<sigrok::Configurable> sr_configurable,
		unsigned int configurable_index,
		const string device_name, const DeviceType device_type):
	sr_configurable_(sr_configurable),
	configurable_index_(configurable_index),
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
	for (const auto &sr_config_key : sr_config_keys) {
		ConfigKey config_key = deviceutil::get_config_key(sr_config_key);
		if (config_key == ConfigKey::Unknown)
			continue;

		qWarning() << "Configurable::init(): Init " << display_name() <<
			" - key " << deviceutil::format_config_key(config_key);

		const auto sr_capabilities =
			sr_configurable_->config_capabilities(sr_config_key);
		if (sr_capabilities.count(sigrok::Capability::GET))
			getable_configs_.insert(config_key);
		if (sr_capabilities.count(sigrok::Capability::SET))
			setable_configs_.insert(config_key);
		if (sr_capabilities.count(sigrok::Capability::LIST))
			listable_configs_.insert(config_key);

		shared_ptr<data::properties::BaseProperty> property;
		const data::DataType data_type =
			deviceutil::get_data_type_for_config_key(config_key);
		switch (data_type) {
		case data::DataType::Int32:
			property = make_shared<data::properties::Int32Property>(
				shared_from_this(), config_key);
			break;
		case data::DataType::UInt64:
			property = make_shared<data::properties::UInt64Property>(
				shared_from_this(), config_key);
			break;
		case data::DataType::Double:
			property = make_shared<data::properties::DoubleProperty>(
				shared_from_this(), config_key);
			break;
		case data::DataType::String:
			property = make_shared<data::properties::StringProperty>(
				shared_from_this(), config_key);
			break;
		case data::DataType::Bool:
			property = make_shared<data::properties::BoolProperty>(
				shared_from_this(), config_key);
			break;
		case data::DataType::MQ:
			property = make_shared<data::properties::MeasuredQuantityProperty>(
				shared_from_this(), config_key);
			break;
		case data::DataType::RationalPeriod:
			property = make_shared<data::properties::RationalProperty>(
				shared_from_this(), config_key);
			break;
		case data::DataType::RationalVolt:
			property = make_shared<data::properties::RationalProperty>(
				shared_from_this(), config_key);
			break;
		case data::DataType::Uint64Range:
			property = make_shared<data::properties::UInt64RangeProperty>(
				shared_from_this(), config_key);
			break;
		case data::DataType::DoubleRange:
			property = make_shared<data::properties::DoubleRangeProperty>(
				shared_from_this(), config_key);
			break;
		case data::DataType::KeyValue:
			// TODO: What is KeyValue?
		case data::DataType::Unknown:
		default:
			assert("Unknown DataType");
		}

		properties_.insert(make_pair(config_key, property));
	}
}

bool Configurable::has_get_config(devices::ConfigKey key)  const
{
	return getable_configs_.count(key);
}

template bool Configurable::get_config(devices::ConfigKey) const;
template int32_t Configurable::get_config(devices::ConfigKey) const;
template uint64_t Configurable::get_config(devices::ConfigKey) const;
template double Configurable::get_config(devices::ConfigKey) const;
template std::string Configurable::get_config(devices::ConfigKey) const;
template<typename T> T Configurable::get_config(devices::ConfigKey config_key) const
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::GET)) {
		qWarning() << "Configurable::get_config(): No getable config key " <<
			devices::deviceutil::format_config_key(config_key);
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
	devices::ConfigKey config_key) const
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::GET)) {
		qWarning() <<
			"Configurable::get_container_config(): No getable config key " <<
			devices::deviceutil::format_config_key(config_key);
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
	return setable_configs_.count(key);
}

template void Configurable::set_config(devices::ConfigKey, const bool);
template void Configurable::set_config(devices::ConfigKey, const int32_t);
template void Configurable::set_config(devices::ConfigKey, const uint64_t);
template void Configurable::set_config(devices::ConfigKey, const double);
template void Configurable::set_config(devices::ConfigKey, const std::string);
template void Configurable::set_config(devices::ConfigKey, const Glib::ustring);
template<typename T> void Configurable::set_config(
	devices::ConfigKey config_key, const T value)
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::SET)) {
		qWarning() << "Configurable::set_config(): No setable  config key  " <<
			devices::deviceutil::format_config_key(config_key);
		assert(false);
	}

	try {
		sr_configurable_->config_set(sr_key, Glib::Variant<T>::create(value));
	}
	catch (sigrok::Error &error) {
		qWarning() << "Configurable::set_config(): Failed to set config key " <<
			devices::deviceutil::format_config_key(config_key) << ". " <<
			error.what();
	}
}

void Configurable::set_container_config(
	devices::ConfigKey config_key, vector<Glib::VariantBase> childs)
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::SET)) {
		qWarning() <<
			"Configurable::set_container_config(): No setable config key  " <<
			devices::deviceutil::format_config_key(config_key);
		assert(false);
	}

	try {
		sr_configurable_->config_set(
			sr_key, Glib::VariantContainerBase::create_tuple(childs));
	}
	catch (sigrok::Error &error) {
		qWarning() <<
			"Configurable::set_container_config(): Failed to set config key " <<
			devices::deviceutil::format_config_key(config_key) << ". " <<
			error.what();
	}
}

bool Configurable::has_list_config(devices::ConfigKey key) const
{
	return listable_configs_.count(key);
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

string Configurable::name() const
{
	string name = "";
	if (auto sr = dynamic_pointer_cast<sigrok::HardwareDevice>(sr_configurable_)) {
		name = "";
	}
	else if (auto sr = dynamic_pointer_cast<sigrok::ChannelGroup>(sr_configurable_)) {
		name = sr->name();
	}
	return name;
}

QString Configurable::display_name() const
{
	QString name = QString::fromStdString(this->name());
	if (name.isEmpty())
		name = QString::fromStdString(device_name_);

	return name;
}

unsigned int Configurable::index() const
{
	return configurable_index_;
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

map<devices::ConfigKey, shared_ptr<data::properties::BaseProperty>>
	Configurable::properties() const
{
	return properties_;
}

shared_ptr<data::properties::BaseProperty>
	Configurable::get_property(devices::ConfigKey config_key) const
{
	if (!properties_.count(config_key))
		return nullptr;
	return properties_.at(config_key);
}

bool Configurable::is_controllable() const
{
	return setable_configs_.size() > 0 ||
		getable_configs_.size() > 0 ||
		listable_configs_.size() > 0;
}

void Configurable::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	for (const auto &entry : sr_meta->config()) {
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
