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

#include <cassert>
#include <type_traits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

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

using std::dynamic_pointer_cast;
using std::make_pair;
using std::make_shared;
using std::set;
using std::string;
using std::vector;

namespace sv {
namespace devices {

Configurable::Configurable(
		const shared_ptr<sigrok::Configurable> sr_configurable,
		unsigned int configurable_index,
		const string &device_name, const DeviceType device_type,
		const QString &device_settings_id):
	sr_configurable_(sr_configurable),
	index_(configurable_index),
	device_name_(device_name),
	device_type_(device_type),
	device_settings_id_(device_settings_id)
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

		qWarning() << "Configurable::init(): Init " << display_name()
			<< " - key " << deviceutil::format_config_key(config_key);

		const auto sr_capabilities =
			sr_configurable_->config_capabilities(sr_config_key);
		if (sr_capabilities.count(sigrok::Capability::GET) > 0)
			getable_configs_.insert(config_key);
		if (sr_capabilities.count(sigrok::Capability::SET) > 0)
			setable_configs_.insert(config_key);
		if (sr_capabilities.count(sigrok::Capability::LIST) > 0)
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
		case data::DataType::RationalVolt:
			property = make_shared<data::properties::RationalProperty>(
				shared_from_this(), config_key);
			break;
		case data::DataType::UInt64Range:
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

		property_map_.insert(make_pair(config_key, property));
	}
}

bool Configurable::has_get_config(devices::ConfigKey config_key)  const
{
	return getable_configs_.count(config_key) > 0;
}

template bool Configurable::get_config(devices::ConfigKey, bool &value) const;
template bool Configurable::get_config(devices::ConfigKey, int32_t &value) const;
template bool Configurable::get_config(devices::ConfigKey, uint64_t &value) const;
template bool Configurable::get_config(devices::ConfigKey, double &value) const;
template bool Configurable::get_config(devices::ConfigKey, std::string &value) const;
template<typename T> bool Configurable::get_config(
	devices::ConfigKey config_key, T &value) const
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::GET)) {
		qWarning() << "Configurable::get_config(): No getable config key "
			<< devices::deviceutil::format_config_key(config_key);
		return false;
	}

	try {
		value = Glib::VariantBase::cast_dynamic<Glib::Variant<T>>(
			sr_configurable_->config_get(sr_key)).get();
	}
	catch (sigrok::Error &error) {
		qWarning() << "Configurable::get_config(): Failed to get key "
			<< devices::deviceutil::format_config_key(config_key) << ". "
			<< error.what();
		return false;
	}

	return true;
}

bool Configurable::get_container_config(
	devices::ConfigKey config_key, Glib::VariantContainerBase &value) const
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::GET)) {
		qWarning()
			<< "Configurable::get_container_config(): No getable config key "
			<< devices::deviceutil::format_config_key(config_key);
		return false;
	}

	try {
		Glib::VariantBase gvar = sr_configurable_->config_get(sr_key);
		if (gvar.is_container()) {
			value =
				Glib::VariantBase::cast_dynamic<Glib::VariantContainerBase>(gvar);
		}
		else {
			value = Glib::VariantContainerBase();
		}
	}
	catch (sigrok::Error &error) {
		qWarning() << "Configurable::get_container_config(): Failed to get key "
			<< devices::deviceutil::format_config_key(config_key) << ". "
			<< error.what();
		return false;
	}

	return true;
}

bool Configurable::get_measured_quantity_config(
	devices::ConfigKey config_key, data::measured_quantity_t &value) const
{
	Glib::VariantContainerBase gvar;
	bool ret = this->get_container_config(config_key, gvar);
	if (!ret)
		return false;

	size_t child_cnt = gvar.get_n_children();
	if (child_cnt != 2) {
		qWarning() << "Configurable::get_measured_quantity_config(): "
			<< "Container (mq) should have 2 child, but has " << child_cnt;
		return false;
	}

	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	uint32_t sr_q =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint32_t>>(gvar).get();
	data::Quantity quantity = data::datautil::get_quantity(sr_q);

	iter.next_value(gvar);
	uint64_t sr_qflags =
		Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(gvar).get();
	set<data::QuantityFlag> quantity_flags =
		data::datautil::get_quantity_flags(sr_qflags);

	value = make_pair(quantity, quantity_flags);

	return true;
}

bool Configurable::has_set_config(devices::ConfigKey config_key) const
{
	return setable_configs_.count(config_key) > 0;
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
		qWarning() << "Configurable::set_config(): No setable config key "
			<< devices::deviceutil::format_config_key(config_key);
		assert(false);
	}

	try {
		sr_configurable_->config_set(sr_key, Glib::Variant<T>::create(value));
	}
	catch (sigrok::Error &error) {
		qWarning() << "Configurable::set_config(): Failed to set config key "
			<< devices::deviceutil::format_config_key(config_key) << ". "
			<< error.what();
	}
}

void Configurable::set_container_config(
	devices::ConfigKey config_key, const vector<Glib::VariantBase> &childs)
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::SET)) {
		qWarning()
			<< "Configurable::set_container_config(): No setable config key "
			<< devices::deviceutil::format_config_key(config_key);
		assert(false);
	}

	try {
		qWarning()
			<< "Configurable::set_container_config(): Set config key "
			<< devices::deviceutil::format_config_key(config_key) << " to "
			<< childs;
		sr_configurable_->config_set(
			sr_key, Glib::VariantContainerBase::create_tuple(childs));
	}
	catch (sigrok::Error &error) {
		qWarning()
			<< "Configurable::set_container_config(): Failed to set config key "
			<< devices::deviceutil::format_config_key(config_key) << ". "
			<< error.what();
	}
}

void Configurable::set_measured_quantity_config(devices::ConfigKey config_key,
	const data::measured_quantity_t &mq)
{
	qWarning()
		<< "Configurable::set_measured_quantity_config(): Set config key "
		<< devices::deviceutil::format_config_key(config_key) << " to "
		<< data::datautil::format_measured_quantity(mq);

	uint32_t sr_q_id = data::datautil::get_sr_quantity_id(mq.first);
	Glib::VariantBase gvar_q = Glib::Variant<uint32_t>::create(sr_q_id);
	uint64_t sr_qfs_id = data::datautil::get_sr_quantity_flags_id(mq.second);
	Glib::VariantBase gvar_qfs = Glib::Variant<uint64_t>::create(sr_qfs_id);

	qWarning()
		<< "Configurable::set_measured_quantity_config(): Set config key "
		<< devices::deviceutil::format_config_key(config_key) << " to "
		<< sr_q_id << ", " << sr_qfs_id;

	vector<Glib::VariantBase> gcontainer;
	gcontainer.push_back(gvar_q);
	gcontainer.push_back(gvar_qfs);

	this->set_container_config(config_key, gcontainer);
}

bool Configurable::has_list_config(devices::ConfigKey config_key) const
{
	return listable_configs_.count(config_key) > 0;
}

bool Configurable::list_config(devices::ConfigKey config_key,
	Glib::VariantContainerBase &gvar)
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::LIST)) {
		qWarning()
			<< "Configurable::list_config(): No config key / no listable config key "
			<< devices::deviceutil::format_config_key(config_key);
		return false;
	}

	try {
		gvar = sr_configurable_->config_list(sr_key);
	}
	catch (sigrok::Error &error) {
		qWarning() << "Configurable::list_config(): Failed to list config key "
			<< devices::deviceutil::format_config_key(config_key) << ". "
			<< error.what();
		return false;
	}

	return true;
}

string Configurable::name() const
{
	string name;
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
	QString display_name = QString::fromStdString(this->name());
	if (display_name.isEmpty())
		display_name = QString::fromStdString(device_name_);

	return display_name;
}

unsigned int Configurable::index() const
{
	return index_;
}

DeviceType Configurable::device_type() const
{
	return device_type_;
}

QString Configurable::device_settings_id() const
{
	return device_settings_id_;
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
	// a configKey is existant in this configurable easier!
	for (const auto &entry : sr_meta->config()) {
		devices::ConfigKey config_key =
			devices::deviceutil::get_config_key(entry.first);

		if (property_map_.count(config_key) == 0) {
			qWarning() << "Configurable::feed_in_meta(): Unknown config key "
				<< QString::fromStdString(entry.first->name())
				<< " for configurable " << display_name() << " received";
			return false;
		}

		property_map_[config_key]->on_value_changed(entry.second);

		// TODO: return QVariant from prop->on_value_changed(); and emit
		//Q_EMIT config_changed(config_key, qvar);
	}

	return true;
}

} // namespace devices
} // namespace sv
