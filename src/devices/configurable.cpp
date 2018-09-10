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
#include "src/devices/properties/floatproperty.hpp"
#include "src/devices/properties/int32property.hpp"
#include "src/devices/properties/measuredquantityproperty.hpp"
#include "src/devices/properties/stringproperty.hpp"
#include "src/devices/properties/uint64property.hpp"

using std::dynamic_pointer_cast;
using std::forward;
using std::get;
using std::is_base_of;
using std::make_pair;
using std::make_shared;
using std::make_tuple;
using std::pair;
using std::string;
using std::tuple;
using sv::devices::ConfigKey;

namespace sv {
namespace devices {

Configurable::Configurable(
		const shared_ptr<sigrok::Configurable> sr_configurable,
		const QString device_name):
	sr_configurable_(sr_configurable),
	device_name_(device_name)
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
		case devices::DataType::Float:
			property = make_shared<properties::FloatProperty>(
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
	assert(key);

	if (getable_configs_.count(key))
		return true;
	return false;
}

template bool Configurable::get_config(devices::ConfigKey) const;
template int32_t Configurable::get_config(devices::ConfigKey) const;
template uint64_t Configurable::get_config(devices::ConfigKey) const;
template double Configurable::get_config(devices::ConfigKey) const;
template std::string Configurable::get_config(devices::ConfigKey) const;
// TODO: This doesn't work: with glibmm >= 2.54.1 but should
template tuple<uint32_t, uint64_t> Configurable::get_config(devices::ConfigKey) const;
//template Configurable::measured_quantity_t Configurable::get_config(devices::ConfigKey) const;
template<typename T> T Configurable::get_config(devices::ConfigKey key) const
{
	assert(key);
	assert(sr_configurable_);

	// Special cases
	/* TODO: move to properties?? Here?? And MQ?
	switch (key) {
	case ConfigKey::OverVoltageProtectionActive:
	case ConfigKey::OverCurrentProtectionActive:
	case ConfigKey::OverTemperatureProtectionActive:
	case ConfigKey::UnderVoltageConditionActive:
		if (getable_configs_.count(key) == 0)
			return false;
		break;
	default:
		break;
	}
	*/

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::GET)) {
		qWarning() << "Configurable::read_config(): No key / no getable key " <<
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

bool Configurable::has_set_config(devices::ConfigKey key) const
{
	assert(key);

	if (setable_configs_.count(key))
		return true;
	return false;
}

template void Configurable::set_config(devices::ConfigKey, const bool);
template void Configurable::set_config(devices::ConfigKey, const int32_t);
template void Configurable::set_config(devices::ConfigKey, const uint64_t);
template void Configurable::set_config(devices::ConfigKey, const double);
template void Configurable::set_config(devices::ConfigKey, const std::string);
// This is working with glibmm < 2.52 (mxe uses glibmm 2.42.0), but libsigrok expects 'r' (this is '{ut}')
template void Configurable::set_config(devices::ConfigKey, const pair<uint32_t, uint64_t>);
// This is working with glibmm >= 2.52 (but mxe uses glibmm 2.42.0). Working with libsigrok (expects 'r')
template void Configurable::set_config(devices::ConfigKey, const tuple<uint32_t, uint64_t>);
// This is for special get/set for measured quantity
//template void Configurable::set_config(devices::ConfigKey, const measured_quantity_t);
template<typename T> void Configurable::set_config(
	devices::ConfigKey key, const T value)
{
	assert(key);
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::SET)) {
		qWarning() << "Configurable::write_config(): No key / no setable key  " <<
			devices::deviceutil::format_config_key(key);
		assert(false);
	}

	try {
		sr_configurable_->config_set(sr_key, Glib::Variant<T>::create(value));
	}
	catch (sigrok::Error &error) {
		qWarning() << "Configurable::list_config(): Failed to set key " <<
			devices::deviceutil::format_config_key(key) << ". " << error.what();
	}
}

bool Configurable::has_list_config(devices::ConfigKey key) const
{
	assert(key);

	if (listable_configs_.count(key))
		return true;
	return false;
}

bool Configurable::list_config(devices::ConfigKey key,
	Glib::VariantContainerBase &gvariant)
{
	assert(key);
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::LIST)) {
		qWarning() << "Configurable::list_config(): No key / no listable key " <<
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

// TODO: Remove from here
bool Configurable::list_config_string_array(devices::ConfigKey key,
	QStringList &string_list)
{
	Glib::VariantContainerBase gvar;
	if (!list_config(key, gvar))
		return false;

	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		string_list.append(QString::fromStdString(
			Glib::VariantBase::cast_dynamic<Glib::Variant<string>>(gvar).get()));
	}

	return true;
}

// TODO: Remove from here, or template for Float, Int32 and UInt64?
bool Configurable::list_config_min_max_step(devices::ConfigKey key,
	double &min, double &max, double &step)
{
	Glib::VariantContainerBase gvar;
	if (!list_config(key, gvar))
		return false;

	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	min = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	max = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	step = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();

	return true;
}


// TODO: Remove from here
bool Configurable::list_config_mq(devices::ConfigKey key,
	measured_quantity_list_t &measured_quantity_list)
{
	Glib::VariantContainerBase gvar;
	if (!list_config(key, gvar))
		return false;

	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		uint32_t mqbits = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint32_t>>(gvar.get_child(0)).get();
		data::Quantity quantity = data::datautil::get_quantity(mqbits);

		if (!measured_quantity_list.count(quantity)) {
			measured_quantity_list.insert(
				make_pair(quantity, vector<set<data::QuantityFlag>>()));
		}

		uint64_t sr_mqflags = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint64_t>>(gvar.get_child(1)).get();
		set<data::QuantityFlag> quantity_flag_set;
		uint64_t mask = 1;
		for (uint i = 0; i < 32; i++, mask <<= 1) {
			if (!(sr_mqflags & mask))
				continue;

			const sigrok::QuantityFlag *sr_mqflag =
				sigrok::QuantityFlag::get(sr_mqflags & mask);
			quantity_flag_set.insert(
				data::datautil::get_quantity_flag(sr_mqflag));
		}
		measured_quantity_list[quantity].push_back(quantity_flag_set);
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

bool Configurable::is_controllable() const
{
	// TODO: Also check for device types?

	if (has_set_config(ConfigKey::Enabled) ||
			has_set_config(ConfigKey::Regulation) ||
			has_set_config(ConfigKey::VoltageTarget) ||
			has_set_config(ConfigKey::CurrentLimit) ||
			has_set_config(ConfigKey::OverVoltageProtectionEnabled) ||
			has_set_config(ConfigKey::OverVoltageProtectionThreshold) ||
			has_set_config(ConfigKey::OverCurrentProtectionEnabled) ||
			has_set_config(ConfigKey::OverCurrentProtectionThreshold) ||
			has_set_config(ConfigKey::UnderVoltageConditionEnabled) ||
			has_set_config(ConfigKey::UnderVoltageConditionThreshold))
		return true;

	if (has_get_config(ConfigKey::MeasuredQuantity) ||
			has_set_config(ConfigKey::MeasuredQuantity))
		return true;

	return false;
}


/**
 * TODO: Remove from here
 * TODO: When glibmm >= 2.52 is more supported and tuple bug is fixed,
 *       use the template function and return tuple<uint32_t, uint64_t>:
 *
 *       return get_config<std::tuple<uint32_t, uint64_t>>(
 *           sigrok::ConfigKey::MEASURED_QUANTITY);
 */
Configurable::measured_quantity_t Configurable::get_measured_quantity() const
{
	if (!sr_configurable_->config_check(
			sigrok::ConfigKey::MEASURED_QUANTITY, sigrok::Capability::GET)) {
		qWarning() << "Configurable::read_config(): " <<
			"No key sigrok::ConfigKey::MEASURED_QUANTITY";
		assert(false);
	}

	auto vb = Glib::VariantBase::cast_dynamic
		<Glib::Variant<std::tuple<uint32_t, uint64_t>>>
		(sr_configurable_->config_get(sigrok::ConfigKey::MEASURED_QUANTITY));

	uint32_t sr_q = vb.get_child<uint32_t>(0);
	data::Quantity qunatity = data::datautil::get_quantity(sr_q);
	uint64_t sr_qfs = vb.get_child<uint64_t>(1);
	set<data::QuantityFlag> quantity_flags =
		data::datautil::get_quantity_flags(sr_qfs);

	return make_pair(qunatity, quantity_flags);
}

/**
 * TODO: Remove from here
 * TODO: This only works with glibmm >= 2.52, we have to change something in
 *       libsigrok to make it work with older distros and MXE (2.42.0)
 */
void Configurable::set_measured_quantity(measured_quantity_t measured_quantity)
{
	uint32_t sr_q_id =
		data::datautil::get_sr_quantity_id(measured_quantity.first);
	uint64_t sr_qfs_id =
		data::datautil::get_sr_quantity_flags_id(measured_quantity.second);

	//auto q_qf_pair = make_pair(sr_q_id, sr_qfs_id); // TODO: Maybe this is a solution?
	auto q_qf_tuple = make_tuple(sr_q_id, sr_qfs_id);

	set_config(ConfigKey::MeasuredQuantity, q_qf_tuple);
}

void Configurable::feed_in_meta(shared_ptr<sigrok::Meta> sr_meta)
{
	for (auto entry : sr_meta->config()) {
		devices::ConfigKey config_key =
			devices::deviceutil::get_config_key(entry.first);

		if (!properties_.count(config_key))
			assert("Unknown config key " <<
				QString::fromStdString(entry.first->name()) << " received");

		properties_[config_key]->on_value_changed(entry.second);

		// TODO: return QVariant from prop->on_value_changed(); and emit
		//Q_EMIT config_changed(config_key, qvar);
	}
}

} // namespace devices
} // namespace sv
