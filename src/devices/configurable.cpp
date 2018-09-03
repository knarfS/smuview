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

#include <cassert>
#include <tuple>
#include <type_traits>
#include <memory>

#include <glib.h>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "configurable.hpp"
#include "src/data/datautil.hpp"

using std::dynamic_pointer_cast;
using std::get;
using std::is_base_of;
using std::make_pair;
using std::make_shared;
using std::make_tuple;
using std::pair;
using std::string;
using std::tuple;

namespace sv {
namespace devices {

Configurable::Configurable(
		const shared_ptr<sigrok::Configurable> sr_configurable,
		const QString device_name):
	sr_configurable_(sr_configurable),
	device_name_(device_name)
{
	init_properties();
	init_values();
}

void Configurable::init_properties()
{
	const auto sr_config_keys = sr_configurable_->config_keys();
	for (auto sr_config_key : sr_config_keys) {
		const auto sr_capabilities =
			sr_configurable_->config_capabilities(sr_config_key);

		if (sr_capabilities.count(sigrok::Capability::GET))
			available_getable_config_keys_.insert(
				devices::deviceutil::get_config_key(sr_config_key));

		if (sr_capabilities.count(sigrok::Capability::SET))
			available_setable_config_keys_.insert(
				devices::deviceutil::get_config_key(sr_config_key));

		if (sr_capabilities.count(sigrok::Capability::LIST))
			available_listable_config_keys_.insert(
				devices::deviceutil::get_config_key(sr_config_key));
	}

	/*
	is_enabled_getable_ = has_get_config(sigrok::ConfigKey::ENABLED);
	is_enabled_setable_ = has_set_config(sigrok::ConfigKey::ENABLED);

	is_regulation_getable_ = has_get_config(sigrok::ConfigKey::REGULATION);
	is_regulation_setable_ = has_set_config(sigrok::ConfigKey::REGULATION);
	is_regulation_listable_ = has_list_config(sigrok::ConfigKey::REGULATION);

	is_voltage_target_getable_ = has_get_config(
		sigrok::ConfigKey::VOLTAGE_TARGET);
	is_voltage_target_setable_ = has_set_config(
		sigrok::ConfigKey::VOLTAGE_TARGET);
	is_voltage_target_listable_ = has_list_config(
		sigrok::ConfigKey::VOLTAGE_TARGET);
	is_current_limit_getable_ = has_get_config(
		sigrok::ConfigKey::CURRENT_LIMIT);
	is_current_limit_setable_ = has_set_config(
		sigrok::ConfigKey::CURRENT_LIMIT);
	is_current_limit_listable_ = has_list_config(
		sigrok::ConfigKey::CURRENT_LIMIT);

	is_otp_active_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE);

	is_ovp_enabled_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED);
	is_ovp_enabled_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED);
	is_ovp_active_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE);
	is_ovp_active_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE);
	is_ovp_threshold_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD);
	is_ovp_threshold_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD);
	is_ovp_threshold_listable_ = has_list_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD);

	is_ocp_enabled_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED);
	is_ocp_enabled_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED);
	is_ocp_active_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE);
	is_ocp_active_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE);
	is_ocp_threshold_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD);
	is_ocp_threshold_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD);
	is_ocp_threshold_listable_ = has_list_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD);

	is_uvc_enabled_getable_ = has_get_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION);
	is_uvc_enabled_setable_ = has_set_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION);
	is_uvc_active_getable_ = has_get_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE);
	is_uvc_active_setable_ = has_set_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE);
	is_uvc_threshold_getable_ = has_get_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
	is_uvc_threshold_setable_ = has_set_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
	is_uvc_threshold_listable_ = has_list_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);

	is_measured_quantity_getable_ = has_get_config(
		sigrok::ConfigKey::MEASURED_QUANTITY);
	is_measured_quantity_setable_ = has_set_config(
		sigrok::ConfigKey::MEASURED_QUANTITY);
	is_measured_quantity_listable_ = has_list_config(
		sigrok::ConfigKey::MEASURED_QUANTITY);

	is_amplitude_getable_ = has_get_config(sigrok::ConfigKey::AMPLITUDE);
	is_amplitude_setable_ = has_set_config(sigrok::ConfigKey::AMPLITUDE);
	is_amplitude_listable_ = has_list_config(sigrok::ConfigKey::AMPLITUDE);
	*/

	/*
	is_offset_getable_ = has_get_config(sigrok::ConfigKey::OFFSET);
	is_offset_setable_ = has_set_config(sigrok::ConfigKey::OFFSET);
	is_offset_listable_ = has_list_config(sigrok::ConfigKey::OFFSET);
	*/
}

void Configurable::init_values()
{
	/*
	if (is_regulation_listable_)
		list_config_string_array(sigrok::ConfigKey::REGULATION,
			regulation_list_);

	if (is_voltage_target_listable_)
		list_config_min_max_steps(sigrok::ConfigKey::VOLTAGE_TARGET,
			voltage_target_min_, voltage_target_max_, voltage_target_step_);

	if (is_current_limit_listable_)
		list_config_min_max_steps(sigrok::ConfigKey::CURRENT_LIMIT,
			current_limit_min_, current_limit_max_, current_limit_step_);

	if (is_ovp_threshold_listable_)
		list_config_min_max_steps(
			sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD,
			ovp_threshold_min_, ovp_threshold_max_, ovp_threshold_step_);

	if (is_ocp_threshold_listable_)
		list_config_min_max_steps(
			sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD,
			ocp_threshold_min_, ocp_threshold_max_, ocp_threshold_step_);

	if (is_uvc_threshold_listable_)
		list_config_min_max_steps(
			sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD,
			uvc_threshold_min_, uvc_threshold_max_, uvc_threshold_step_);

	if (is_measured_quantity_listable_)
		list_config_mq(
			sigrok::ConfigKey::MEASURED_QUANTITY,
			measured_quantity_list_);

	if (is_amplitude_listable_)
		list_config_min_max_steps(sigrok::ConfigKey::AMPLITUDE,
		amplitude_min_, amplitude_max_, amplitude_step_);
	*/

	/*
	if (is_offset_listable_)
		list_config_min_max_steps(sigrok::ConfigKey::OFFSET,
		offset_min_, offset_max_, offset_step_);
	*/
}

bool Configurable::has_get_config(devices::ConfigKey key)  const
{
	assert(key);

	if (available_getable_config_keys_.count(key))
		return true;
	return false;
}

template bool Configurable::get_config(devices::ConfigKey) const;
template uint64_t Configurable::get_config(devices::ConfigKey) const;
template double Configurable::get_config(devices::ConfigKey) const;
// TODO: This doesn't work: with glibmm >= 2.54.1 but should
//template tuple<uint32_t, uint64_t> Configurable::get_config(devices::ConfigKey) const;
template<typename T> T Configurable::get_config(devices::ConfigKey key) const
{
	assert(key);
	assert(sr_configurable_);

	/*
	try {
		auto gvar = sr_configurable_->config_get(sigrok::ConfigKey::ENABLED);
		enable =
			Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(gvar).get();
	} catch (sigrok::Error error) {
		qDebug() << "Failed to get ENABLED.";
		return false;
	}
	*/

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::GET)) {
		qWarning() << "Configurable::read_config(): No key " <<
			QString::fromStdString(sr_key->name());
		assert(false);
	}

	return Glib::VariantBase::cast_dynamic<Glib::Variant<T>>(
		sr_configurable_->config_get(sr_key)).get();
}

bool Configurable::has_set_config(devices::ConfigKey key) const
{
	assert(key);

	if (available_setable_config_keys_.count(key))
		return true;
	return false;
}

template void Configurable::set_config(devices::ConfigKey, const bool);
template void Configurable::set_config(devices::ConfigKey, const uint64_t);
template void Configurable::set_config(devices::ConfigKey, const double);
// This is working with glibmm < 2.52 (mxe uses glibmm 2.42.0), but libsigrok expects 'r' (this is '{ut}')
template void Configurable::set_config(devices::ConfigKey, const pair<uint32_t, uint64_t>);
// This is working with glibmm >= 2.52 (but mxe uses glibmm 2.42.0). Working with libsigrok (expects 'r')
template void Configurable::set_config(devices::ConfigKey, const tuple<uint32_t, uint64_t>);
template<typename T> void Configurable::set_config(
	devices::ConfigKey key, const T value)
{
	assert(key);
	assert(sr_configurable_);

	/*
	try {
		sr_configurable_->config_set(
			sigrok::ConfigKey::ENABLED, Glib::Variant<bool>::create(enable));
	} catch (sigrok::Error error) {
		qDebug() << "Failed to set ENABLED.";
	}
	*/

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::SET)) {
		qWarning() << "Configurable::write_config(): No key " <<
			QString::fromStdString(sr_key->name());
		assert(false);
	}

	sr_configurable_->config_set(sr_key, Glib::Variant<T>::create(value));
}

bool Configurable::has_list_config(devices::ConfigKey key) const
{
	assert(key);

	if (available_listable_config_keys_.count(key))
		return true;
	return false;
}

void Configurable::list_config_string_array(devices::ConfigKey key,
	QStringList &string_list)
{
	assert(key);
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::LIST)) {
		qWarning() << "Configurable::list_config_string_array(): No key " <<
			QString::fromStdString(sr_key->name());
		assert(false);
	}

	Glib::VariantContainerBase gvar = sr_configurable_->config_list(sr_key);
	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		string_list.append(QString::fromStdString(
			Glib::VariantBase::cast_dynamic<Glib::Variant<string>>(gvar).get()));
	}
}

void Configurable::list_config_min_max_steps(devices::ConfigKey key,
	double &min, double &max, double &step)
{
	assert(key);
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::LIST)) {
		qWarning() << "Configurable::list_config_min_max_steps(): No key " <<
			QString::fromStdString(sr_key->name()) << " or not listable!";
		assert(false);
	}

	Glib::VariantContainerBase gvar = sr_configurable_->config_list(sr_key);
	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	min = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	max = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	step = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
}

void Configurable::list_config_mq(devices::ConfigKey key,
	measured_quantity_list_t &measured_quantity_list)
{
	assert(key);
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::LIST)) {
		qWarning() << "Configurable::list_config_mq(): No key "
			<< QString::fromStdString(sr_key->name()) << " or not listable!";
		assert(false);
	}

	Glib::VariantContainerBase gvar = sr_configurable_->config_list(sr_key);
	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		uint32_t mqbits = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint32_t>>(gvar.get_child(0)).get();
		data::Quantity quantity = data::quantityutil::get_quantity(mqbits);

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
				data::quantityutil::get_quantity_flag(sr_mqflag));
		}
		measured_quantity_list[quantity].push_back(quantity_flag_set);
	}
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

set<devices::ConfigKey> Configurable::available_getable_config_keys() const
{
	return available_getable_config_keys_;
}

set<devices::ConfigKey> Configurable::available_setable_config_keys() const
{
	return available_setable_config_keys_;
}

set<devices::ConfigKey> Configurable::available_listable_config_keys() const
{
	return available_listable_config_keys_;
}

/*
bool Configurable::get_enabled() const
{
	return get_config<bool>(sigrok::ConfigKey::ENABLED);
}

void Configurable::set_enabled(const bool enabled)
{
	set_config(sigrok::ConfigKey::ENABLED, enabled);
}


bool Configurable::get_regulation() const
{
	return get_config<bool>(sigrok::ConfigKey::REGULATION);
}

void Configurable::set_regulation(const bool regulation)
{
	set_config(sigrok::ConfigKey::REGULATION, regulation);
}


double Configurable::get_voltage_target() const
{
	return get_config<double>(sigrok::ConfigKey::VOLTAGE_TARGET);
}

void Configurable::set_voltage_target(const double value)
{
	set_config(sigrok::ConfigKey::VOLTAGE_TARGET, value);
}


double Configurable::get_current_limit() const
{
	return get_config<double>(sigrok::ConfigKey::CURRENT_LIMIT);
}

void Configurable::set_current_limit(const double value)
{
	set_config(sigrok::ConfigKey::CURRENT_LIMIT, value);
}


bool Configurable::get_ovp_enabled() const
{
	return get_config<bool>(sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED);
}

void Configurable::set_ovp_enabled(const bool enabled)
{
	set_config(sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED, enabled);
}

bool Configurable::get_ovp_active() const
{
	if (is_ovp_active_getable_)
		return get_config<bool>(
			sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE);
	else
		return false;
}

double Configurable::get_ovp_threshold() const
{
	return get_config<double>(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD);
}

void Configurable::set_ovp_threshold(const double threshold)
{
	set_config(
		sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD, threshold);
}


bool Configurable::get_ocp_enabled() const
{
	return get_config<bool>(sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED);
}

void Configurable::set_ocp_enabled(const bool enabled)
{
	set_config(sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED, enabled);
}

bool Configurable::get_ocp_active() const
{
	if (is_ocp_active_getable_)
		return get_config<bool>(
			sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE);
	else
		return false;
}

double Configurable::get_ocp_threshold() const
{
	return get_config<double>(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD);
}

void Configurable::set_ocp_threshold(const double threshold)
{
	set_config(
		sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD, threshold);
}


bool Configurable::get_otp_active() const
{
	if (is_otp_active_getable_)
		return get_config<bool>(
			sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE);
	else
		return false;
}


bool Configurable::get_uvc_enabled() const
{
	return get_config<bool>(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION);
}

void Configurable::set_uvc_enabled(const bool enabled)
{
	set_config(sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION, enabled);
}

bool Configurable::get_uvc_active() const
{
	if (is_uvc_active_getable_)
		return get_config<bool>(
			sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE);
	else
		return false;
}

double Configurable::get_uvc_threshold() const
{
	return get_config<double>(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
}

void Configurable::set_uvc_threshold(const double threshold)
{
	set_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD, threshold);
}
*/


/**
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
	data::Quantity qunatity = data::quantityutil::get_quantity(sr_q);
	uint64_t sr_qfs = vb.get_child<uint64_t>(1);
	set<data::QuantityFlag> quantity_flags =
		data::quantityutil::get_quantity_flags(sr_qfs);

	return make_pair(qunatity, quantity_flags);
}

/**
 * TODO: This only works with glibmm >= 2.52, we have to change something in
 *       libsigrok to make it work with older distros and MXE (2.42.0)
 */
void Configurable::set_measured_quantity(measured_quantity_t measured_quantity)
{
	uint32_t sr_q_id =
		data::quantityutil::get_sr_quantity_id(measured_quantity.first);
	uint64_t sr_qfs_id =
		data::quantityutil::get_sr_quantity_flags_id(measured_quantity.second);

	//auto q_qf_pair = make_pair(sr_q_id, sr_qfs_id); // TODO: Maybe this is a solution?
	auto q_qf_tuple = make_tuple(sr_q_id, sr_qfs_id);

	set_config(ConfigKey::MeasuredQuantity, q_qf_tuple);
}


/*
double Configurable::get_amplitude() const
{
	return get_config<double>(sigrok::ConfigKey::AMPLITUDE);
}

void Configurable::set_amplitude(double amplitude)
{
	set_config(sigrok::ConfigKey::AMPLITUDE, amplitude);
}


double Configurable::get_offset() const
{
	/ *
	return get_config<double>(sigrok::ConfigKey::OFFSET);
	* /
	return 0;
}

void Configurable::set_offset(double offset)
{
	(void)offset;
	/ *
	set_config(sigrok::ConfigKey::OFFSET, offset);
	* /
}


bool Configurable::list_regulation(QStringList &regulation_list)
{
	if (!is_regulation_listable_)
		return false;

	regulation_list = regulation_list_;
	return true;
}

bool Configurable::list_voltage_target(double &min, double &max, double &step)
{
	if (!is_voltage_target_listable_)
		return false;

	min = voltage_target_min_;
	max = voltage_target_max_;
	step = voltage_target_step_;
	return true;
}

bool Configurable::list_current_limit(double &min, double &max, double &step)
{
	if (!is_current_limit_listable_)
		return false;

	min = current_limit_min_;
	max = current_limit_max_;
	step = current_limit_step_;
	return true;
}

bool Configurable::list_ovp_threshold(double &min, double &max, double &step)
{
	if (!is_ovp_threshold_listable_)
		return false;

	min = ovp_threshold_min_;
	max = ovp_threshold_max_;
	step = ovp_threshold_step_;
	return true;
}

bool Configurable::list_ocp_threshold(double &min, double &max, double &step)
{
	if (!is_ocp_threshold_listable_)
		return false;

	min = ocp_threshold_min_;
	max = ocp_threshold_max_;
	step = ocp_threshold_step_;
	return true;
}

bool Configurable::list_uvc_threshold(double &min, double &max, double &step)
{
	if (!is_uvc_threshold_listable_)
		return false;

	min = uvc_threshold_min_;
	max = uvc_threshold_max_;
	step = uvc_threshold_step_;
	return true;
}

bool Configurable::list_measured_quantity(
	measured_quantity_list_t &measured_quantity_list)
{
	if (!is_measured_quantity_listable_)
		return false;

	measured_quantity_list = measured_quantity_list_;
	return true;
}

bool Configurable::list_amplitude(double &min, double &max, double &step)
{
	if (!is_amplitude_listable_)
		return false;

	min = amplitude_min_;
	max = amplitude_max_;
	step = amplitude_step_;
	return true;
}

bool Configurable::list_offset(double &min, double &max, double &step)
{
	if (!is_offset_listable_)
		return false;

	min = offset_min_;
	max = offset_max_;
	step = offset_step_;
	return true;
}
*/

bool Configurable::is_controllable() const
{
	/* TODO: device types
	if (type_ == Type::POWER_SUPPLY || type_ == Type::ELECTRONIC_LOAD) {
		if (is_enabled_setable_ || is_regulation_setable_ ||
				is_voltage_target_setable_ || is_current_limit_setable_ ||
				is_ovp_enabled_setable_ || is_ovp_threshold_setable_ ||
				is_ocp_enabled_setable_ || is_ocp_threshold_setable_ ||
				is_uvc_enabled_setable_ || is_uvc_threshold_setable_)
			return true;
	}
	else if (type_ == Type::MULTIMETER) {
		if (is_measured_quantity_getable_ || is_measured_quantity_setable_)
			return true;
	}
	*/

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


/*
bool Configurable::is_enabled_getable() const
{
	return is_enabled_getable_;
}

bool Configurable::is_enabled_setable() const
{
	return is_enabled_setable_;
}


bool Configurable::is_regulation_getable() const
{
	return is_regulation_getable_;
}

bool Configurable::is_regulation_setable() const
{
	return is_regulation_setable_;
}


bool Configurable::is_voltage_target_getable() const
{
	return is_voltage_target_getable_;
}

bool Configurable::is_voltage_target_setable() const
{
	return is_voltage_target_setable_;
}

bool Configurable::is_voltage_target_listable() const
{
	return is_voltage_target_listable_;
}


bool Configurable::is_current_limit_getable() const
{
	return is_current_limit_getable_;
}

bool Configurable::is_current_limit_setable() const
{
	return is_current_limit_setable_;
}

bool Configurable::is_current_limit_listable() const
{
	return is_current_limit_listable_;
}


bool Configurable::is_otp_active_getable() const
{
	return is_otp_active_getable_;
}


bool Configurable::is_ovp_enabled_getable() const
{
	return is_ovp_enabled_getable_;
}

bool Configurable::is_ovp_enabled_setable() const
{
	return is_ovp_enabled_setable_;
}

bool Configurable::is_ovp_active_getable() const
{
	return is_ovp_active_getable_;
}

bool Configurable::is_ovp_threshold_getable() const
{
	return is_ovp_threshold_getable_;
}

bool Configurable::is_ovp_threshold_setable() const
{
	return is_ovp_threshold_setable_;
}

bool Configurable::is_ovp_threshold_listable() const
{
	return is_ovp_threshold_listable_;
}


bool Configurable::is_ocp_enabled_getable() const
{
	return is_ocp_enabled_getable_;
}

bool Configurable::is_ocp_enabled_setable() const
{
	return is_ocp_enabled_setable_;
}

bool Configurable::is_ocp_active_getable() const
{
	return is_ocp_active_getable_;
}

bool Configurable::is_ocp_threshold_getable() const
{
	return is_ocp_threshold_getable_;
}

bool Configurable::is_ocp_threshold_setable() const
{
	return is_ocp_threshold_setable_;
}

bool Configurable::is_ocp_threshold_listable() const
{
	return is_ocp_threshold_listable_;
}


bool Configurable::is_uvc_enabled_getable() const
{
	return is_uvc_enabled_getable_;
}

bool Configurable::is_uvc_enabled_setable() const
{
	return is_uvc_enabled_setable_;
}

bool Configurable::is_uvc_active_getable() const
{
	return is_uvc_active_getable_;
}

bool Configurable::is_uvc_threshold_getable() const
{
	return is_uvc_threshold_getable_;
}

bool Configurable::is_uvc_threshold_setable() const
{
	return is_uvc_threshold_setable_;
}

bool Configurable::is_uvc_threshold_listable() const
{
	return is_uvc_threshold_listable_;
}


bool Configurable::is_measured_quantity_getable() const
{
	return is_measured_quantity_getable_;
}

bool Configurable::is_measured_quantity_setable() const
{
	return is_measured_quantity_setable_;
}

bool Configurable::is_measured_quantity_listable() const
{
	return is_measured_quantity_listable_;
}

bool Configurable::is_amplitude_getable() const
{
	return is_amplitude_getable_;
}

bool Configurable::is_amplitude_setable() const
{
	return is_amplitude_setable_;
}

bool Configurable::is_amplitude_listable() const
{
	return is_amplitude_listable_;
}

bool Configurable::is_offset_getable() const
{
	return is_offset_getable_;
}

bool Configurable::is_offset_setable() const
{
	return is_offset_setable_;
}

bool Configurable::is_offset_listable() const
{
	return is_offset_listable_;
}
*/

} // namespace devices
} // namespace sv
