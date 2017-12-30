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
#include <type_traits>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "configurable.hpp"
#include "src/util.hpp"

using std::dynamic_pointer_cast;
using std::is_base_of;
using std::make_shared;
using std::pair;
using std::string;

namespace sv {
namespace devices {

Configurable::Configurable(const shared_ptr<sigrok::Configurable> sr_configurable):
	sr_configurable_(sr_configurable)
{
	init_properties();
	init_values();
}

void Configurable::init_properties()
{
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

	is_otp_enabled_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION);
	is_otp_enabled_setable_ = has_set_config(
		sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION);
	is_otp_active_getable_ = has_get_config(
		sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE);
	is_otp_active_setable_ = has_set_config(
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
	/*
	is_uvc_threshold_getable_ = has_get_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
	is_uvc_threshold_setable_ = has_set_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
	is_uvc_threshold_listable_ = has_list_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
	*/
	is_uvc_threshold_getable_ = false;
	is_uvc_threshold_setable_ = false;
	is_uvc_threshold_listable_ = false;

	is_measured_quantity_getable_ = has_get_config(
		sigrok::ConfigKey::MEASURED_QUANTITY);
	is_measured_quantity_setable_ = has_set_config(
		sigrok::ConfigKey::MEASURED_QUANTITY);
	is_measured_quantity_listable_ = has_list_config(
		sigrok::ConfigKey::MEASURED_QUANTITY);
}

void Configurable::init_values()
{
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

	/*
	if (is_uvc_threshold_listable_)
		list_config_min_max_steps(
			sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD,
			uvc_threshold_min_, uvc_threshold_max_, uvc_threshold_step_);
	*/

	if (is_measured_quantity_listable_)
		list_config_mq(sigrok::ConfigKey::MEASURED_QUANTITY,
			sr_mq_flags_list_, mq_flags_list_);
}

bool Configurable::has_get_config(const sigrok::ConfigKey *key)  const
{
	assert(key);
	assert(sr_configurable_);

	if (!sr_configurable_->config_check(key, sigrok::Capability::GET))
		return false;

	return true;
}

template bool Configurable::get_config(const sigrok::ConfigKey*) const;
template uint64_t Configurable::get_config(const sigrok::ConfigKey*) const;
template double Configurable::get_config(const sigrok::ConfigKey*) const;
template<typename T> T Configurable::get_config(const sigrok::ConfigKey *key) const
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

	if (!sr_configurable_->config_check(key, sigrok::Capability::GET)) {
		qWarning() << "Configurable::read_config(): No key " <<
			QString::fromStdString(key->name());
		assert(false);
	}

	return Glib::VariantBase::cast_dynamic<Glib::Variant<T>>(
		sr_configurable_->config_get(key)).get();
}

bool Configurable::has_set_config(const sigrok::ConfigKey *key) const
{
	assert(key);
	assert(sr_configurable_);

	if (!sr_configurable_->config_check(key, sigrok::Capability::SET))
		return false;

	return true;
}

template void Configurable::set_config(const sigrok::ConfigKey*, const bool);
template void Configurable::set_config(const sigrok::ConfigKey*, const uint64_t);
template void Configurable::set_config(const sigrok::ConfigKey*, const double);
template<typename T> void Configurable::set_config(
		const sigrok::ConfigKey *key, const T value)
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

	if (!sr_configurable_->config_check(key, sigrok::Capability::SET)) {
		qWarning() << "Configurable::write_config(): No key " <<
			QString::fromStdString(key->name());
		assert(false);
	}

	sr_configurable_->config_set(key, Glib::Variant<T>::create(value));
}

bool Configurable::has_list_config(const sigrok::ConfigKey *key) const
{
	assert(key);
	assert(sr_configurable_);

	if (!sr_configurable_->config_check(key, sigrok::Capability::LIST))
		return false;

	return true;
}

void Configurable::list_config_string_array(const sigrok::ConfigKey *key,
	QStringList &string_list)
{
	assert(key);
	assert(sr_configurable_);

	if (!sr_configurable_->config_check(key, sigrok::Capability::LIST)) {
		qWarning() << "Configurable::list_config_string_array(): No key " <<
			QString::fromStdString(key->name());
		assert(false);
	}

	Glib::VariantContainerBase gvar = sr_configurable_->config_list(key);
	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		string_list.append(QString::fromStdString(
			Glib::VariantBase::cast_dynamic<Glib::Variant<string>>(gvar).get()));
	}
}

void Configurable::list_config_min_max_steps(const sigrok::ConfigKey *key,
	double &min, double &max, double &step)
{
	assert(key);
	assert(sr_configurable_);

	if (!sr_configurable_->config_check(key, sigrok::Capability::LIST)) {
		qWarning() << "Configurable::list_config_min_max_steps(): No key " <<
			QString::fromStdString(key->name());
		assert(false);
	}

	Glib::VariantContainerBase gvar = sr_configurable_->config_list(key);
	Glib::VariantIter iter(gvar);
	iter.next_value(gvar);
	min = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	max = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
	iter.next_value(gvar);
	step = Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(gvar).get();
}

void Configurable::list_config_mq(const sigrok::ConfigKey *key,
	sr_mq_flags_list_t &sr_mq_flags_list, mq_flags_list_t &mq_flags_list)
{
	assert(key);
	assert(sr_configurable_);

	if (!sr_configurable_->config_check(key, sigrok::Capability::LIST)) {
		qWarning() << "Configurable::list_config_mq(): No key " <<
			QString::fromStdString(key->name());
		assert(false);
	}

	Glib::VariantContainerBase gvar = sr_configurable_->config_list(key);
	Glib::VariantIter iter(gvar);
	while (iter.next_value (gvar)) {
		uint32_t mqbits = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint32_t>>(gvar.get_child(0)).get();
		const sigrok::Quantity *sr_mq = sigrok::Quantity::get(mqbits);
		QString mq = util::format_quantity(sr_mq);

		// TODO Das geht besser....
		shared_ptr<vector<set<const sigrok::QuantityFlag *>>> sr_flag_vector;
		shared_ptr<vector<set<QString>>> flag_vector;
		if (!sr_mq_flags_list.count(sr_mq)) {
			sr_flag_vector = make_shared<vector<set<const sigrok::QuantityFlag *>>>();
			flag_vector = make_shared<vector<set<QString>>>();
			sr_mq_flags_list.insert(
				pair<const sigrok::Quantity *, shared_ptr<vector<set<const sigrok::QuantityFlag *>>>>
				(sr_mq, sr_flag_vector));
			mq_flags_list.insert(
				pair<QString, shared_ptr<vector<set<QString>>>>
				(mq, flag_vector));
		}
		else {
			sr_flag_vector = sr_mq_flags_list[sr_mq];
			flag_vector = mq_flags_list[mq];
		}

		uint64_t sr_mqflags = Glib::VariantBase::cast_dynamic
			<Glib::Variant<uint64_t>>(gvar.get_child(1)).get();

		set<const sigrok::QuantityFlag *> sr_flag_set;
		set<QString> flag_set;
		uint64_t mask = 1;
		for (uint i = 0; i < 32; i++, mask <<= 1) {
			if (!(sr_mqflags & mask))
				continue;

			const sigrok::QuantityFlag *sr_mqflag =
				sigrok::QuantityFlag::get(sr_mqflags & mask);
			QString mqflag = util::format_quantityflag(sr_mqflag);

			sr_flag_set.insert(sr_mqflag);
			flag_set.insert(mqflag);
		}
		sr_flag_vector->push_back(sr_flag_set);
		flag_vector->push_back(flag_set);
	}
}

QString Configurable::name() const
{
	QString name("?");
	if (auto sr = dynamic_pointer_cast<sigrok::HardwareDevice>(sr_configurable_)) {
		name = QString::fromStdString(sr->model());
	}
	else if (auto sr = dynamic_pointer_cast<sigrok::ChannelGroup>(sr_configurable_)) {
		name = QString::fromStdString(sr->name());
	}
	return name;
}

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
	/*
	return get_config<double>(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD);
	*/
	return 0;
}

void Configurable::set_uvc_threshold(const double threshold)
{
	(void)threshold;

	/*
	set_config(
		sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD, threshold);
	*/
}


void Configurable::get_measured_quantity() const
{
	//get_config<double>(sigrok::ConfigKey::MEASURED_QUANTITY);
}

void Configurable::set_measured_quantity(uint mq, uint mq_flags)
{
	(void)mq;
	(void)mq_flags;

	//set_config(sigrok::ConfigKey::MEASURED_QUANTITY, mq);
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
	sr_mq_flags_list_t &sr_mq_flags_list, mq_flags_list_t &mq_flags_list)
{
	if (!is_measured_quantity_listable_)
		return false;

	sr_mq_flags_list = sr_mq_flags_list_;
	mq_flags_list = mq_flags_list_;
	return true;
}


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

	if (is_enabled_setable_ || is_regulation_setable_ ||
			is_voltage_target_setable_ || is_current_limit_setable_ ||
			is_ovp_enabled_setable_ || is_ovp_threshold_setable_ ||
			is_ocp_enabled_setable_ || is_ocp_threshold_setable_ ||
			is_uvc_enabled_setable_ || is_uvc_threshold_setable_)
		return true;

	if (is_measured_quantity_getable_ || is_measured_quantity_setable_)
		return true;

	return false;
}


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


bool Configurable::is_otp_active_getable() const
{
	return is_otp_active_getable_;
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

} // namespace devices
} // namespace sv
