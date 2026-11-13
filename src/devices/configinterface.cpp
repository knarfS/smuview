/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
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
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <glib.h>
#include <glibmm.h>

#include <QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "configinterface.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/deviceutil.hpp"

using std::make_pair;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {
namespace devices {

ConfigInterface::ConfigInterface(
		const shared_ptr<sigrok::Configurable> sr_configurable) :
	sr_configurable_(sr_configurable)
{
}

bool ConfigInterface::has_get_config(devices::ConfigKey config_key) const
{
	return sr_configurable_->config_check(
		deviceutil::get_sr_config_key(config_key), sigrok::Capability::GET);
}

template bool ConfigInterface::get_config(devices::ConfigKey, bool &value) const;
template bool ConfigInterface::get_config(devices::ConfigKey, int32_t &value) const;
template bool ConfigInterface::get_config(devices::ConfigKey, uint64_t &value) const;
template bool ConfigInterface::get_config(devices::ConfigKey, double &value) const;
template bool ConfigInterface::get_config(devices::ConfigKey, std::string &value) const;
template<typename T> bool ConfigInterface::get_config(
	devices::ConfigKey config_key, T &value) const
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::GET)) {
		qWarning() << "ConfigInterface::get_config(): No getable config key "
			<< devices::deviceutil::format_config_key(config_key);
		return false;
	}

	try {
		value = Glib::VariantBase::cast_dynamic<Glib::Variant<T>>(
			sr_configurable_->config_get(sr_key)).get();
	}
	catch (sigrok::Error &error) {
		qWarning() << "ConfigInterface::get_config(): Failed to get key "
			<< devices::deviceutil::format_config_key(config_key) << ". "
			<< error.what();
		return false;
	}

	return true;
}

bool ConfigInterface::get_container_config(
	devices::ConfigKey config_key, Glib::VariantContainerBase &value) const
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::GET)) {
		qWarning()
			<< "ConfigInterface::get_container_config(): No getable config key "
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
		qWarning() << "ConfigInterface::get_container_config(): Failed to get key "
			<< devices::deviceutil::format_config_key(config_key) << ". "
			<< error.what();
		return false;
	}

	return true;
}

bool ConfigInterface::get_measured_quantity_config(
	devices::ConfigKey config_key, data::measured_quantity_t &value) const
{
	Glib::VariantContainerBase gvar;
	bool ret = this->get_container_config(config_key, gvar);
	if (!ret)
		return false;

	size_t child_cnt = gvar.get_n_children();
	if (child_cnt != 2) {
		qWarning() << "ConfigInterface::get_measured_quantity_config(): "
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

bool ConfigInterface::has_set_config(devices::ConfigKey config_key) const
{
	return sr_configurable_->config_check(
		deviceutil::get_sr_config_key(config_key), sigrok::Capability::SET);
}

template void ConfigInterface::set_config(devices::ConfigKey, const bool);
template void ConfigInterface::set_config(devices::ConfigKey, const int32_t);
template void ConfigInterface::set_config(devices::ConfigKey, const uint64_t);
template void ConfigInterface::set_config(devices::ConfigKey, const double);
template void ConfigInterface::set_config(devices::ConfigKey, const std::string);
template void ConfigInterface::set_config(devices::ConfigKey, const Glib::ustring);
template<typename T> void ConfigInterface::set_config(
	devices::ConfigKey config_key, const T value)
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::SET)) {
		qWarning() << "ConfigInterface::set_config(): No setable config key "
			<< devices::deviceutil::format_config_key(config_key);
		assert(false);
	}

	try {
		sr_configurable_->config_set(sr_key, Glib::Variant<T>::create(value));
	}
	catch (sigrok::Error &error) {
		qWarning() << "ConfigInterface::set_config(): Failed to set config key "
			<< devices::deviceutil::format_config_key(config_key) << ". "
			<< error.what();
	}
}

void ConfigInterface::set_container_config(
	devices::ConfigKey config_key, const vector<Glib::VariantBase> &childs)
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::SET)) {
		qWarning()
			<< "ConfigInterface::set_container_config(): No setable config key "
			<< devices::deviceutil::format_config_key(config_key);
		assert(false);
	}

	try {
		qWarning()
			<< "ConfigInterface::set_container_config(): Set config key "
			<< devices::deviceutil::format_config_key(config_key) << " to "
			<< childs;
		sr_configurable_->config_set(
			sr_key, Glib::VariantContainerBase::create_tuple(childs));
	}
	catch (sigrok::Error &error) {
		qWarning()
			<< "ConfigInterface::set_container_config(): Failed to set config key "
			<< devices::deviceutil::format_config_key(config_key) << ". "
			<< error.what();
	}
}

void ConfigInterface::set_measured_quantity_config(devices::ConfigKey config_key,
	const data::measured_quantity_t &mq)
{
	qWarning()
		<< "ConfigInterface::set_measured_quantity_config(): Set config key "
		<< devices::deviceutil::format_config_key(config_key) << " to "
		<< data::datautil::format_measured_quantity(mq);

	uint32_t sr_q_id = data::datautil::get_sr_quantity_id(mq.first);
	Glib::VariantBase gvar_q = Glib::Variant<uint32_t>::create(sr_q_id);
	uint64_t sr_qfs_id = data::datautil::get_sr_quantity_flags_id(mq.second);
	Glib::VariantBase gvar_qfs = Glib::Variant<uint64_t>::create(sr_qfs_id);

	qWarning()
		<< "ConfigInterface::set_measured_quantity_config(): Set config key "
		<< devices::deviceutil::format_config_key(config_key) << " to "
		<< sr_q_id << ", " << sr_qfs_id;

	vector<Glib::VariantBase> gcontainer;
	gcontainer.push_back(gvar_q);
	gcontainer.push_back(gvar_qfs);

	this->set_container_config(config_key, gcontainer);
}

bool ConfigInterface::has_list_config(devices::ConfigKey config_key) const
{
	return sr_configurable_->config_check(
		deviceutil::get_sr_config_key(config_key), sigrok::Capability::LIST);
}

bool ConfigInterface::list_config(devices::ConfigKey config_key,
	Glib::VariantContainerBase &gvar)
{
	assert(sr_configurable_);

	const sigrok::ConfigKey *sr_key =
		devices::deviceutil::get_sr_config_key(config_key);

	if (!sr_configurable_->config_check(sr_key, sigrok::Capability::LIST)) {
		qWarning()
			<< "ConfigInterface::list_config(): No config key / no listable config key "
			<< devices::deviceutil::format_config_key(config_key);
		return false;
	}

	try {
		gvar = sr_configurable_->config_list(sr_key);
	}
	catch (sigrok::Error &error) {
		qWarning() << "ConfigInterface::list_config(): Failed to list config key "
			<< devices::deviceutil::format_config_key(config_key) << ". "
			<< error.what();
		return false;
	}

	return true;
}

} // namespace devices
} // namespace sv
