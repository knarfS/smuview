/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <tuple>
#include <utility>

#include <QDebug>

#include "measuredquantityproperty.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/configurable.hpp"

using std::make_pair;
using std::make_tuple;
using std::tuple;

Q_DECLARE_METATYPE(sv::devices::Configurable::measured_quantity_t)

namespace sv {
namespace devices {
namespace properties {

MeasuredQuantityProperty::MeasuredQuantityProperty(
		shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key)
{
	if (is_listable_) {
		configurable_->list_config_string_array(config_key_, list_values_);
	}
}

QVariant MeasuredQuantityProperty::value() const
{
	return QVariant().fromValue(measured_quantity_value());
}

/**
 * TODO: When glibmm >= 2.52 is more supported and tuple bug is fixed,
 *       use the template function and return tuple<uint32_t, uint64_t>:
 *
 *       return get_config<std::tuple<uint32_t, uint64_t>>(
 *           sigrok::ConfigKey::MEASURED_QUANTITY);
 */
Configurable::measured_quantity_t
MeasuredQuantityProperty::measured_quantity_value() const
{
	tuple<uint32_t, uint64_t> sr_mq = configurable_->
		get_config<tuple<uint32_t, uint64_t>>(config_key_);

	uint32_t sr_q = std::get<0>(sr_mq);
	data::Quantity quantity = data::datautil::get_quantity(sr_q);
	uint64_t sr_qf = std::get<1>(sr_mq);
	set<data::QuantityFlag> quantity_flags =
		data::datautil::get_quantity_flags(sr_qf);

	return make_pair(quantity, quantity_flags);
}

Configurable::measured_quantity_list_t
MeasuredQuantityProperty::list_values() const
{
	Configurable::measured_quantity_list_t measured_quantity_list;

	Glib::VariantContainerBase gvar;
	if (!configurable_->list_config(config_key_, gvar))
		return measured_quantity_list;

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

	return measured_quantity_list;
}

/**
 * TODO: This only works with glibmm >= 2.52, we have to change something in
 *       libsigrok to make it work with older distros and MXE (2.42.0)
 */
void MeasuredQuantityProperty::change_value(const QVariant qvar)
{
	Configurable::measured_quantity_t mq =
		qvar.value<Configurable::measured_quantity_t>();

	uint32_t sr_q_id = data::datautil::get_sr_quantity_id(mq.first);
	uint64_t sr_qfs_id = data::datautil::get_sr_quantity_flags_id(mq.second);

	//auto q_qf_pair = make_pair(sr_q_id, sr_qfs_id); // TODO: Maybe this is a solution?
	auto q_qf_tuple = make_tuple(sr_q_id, sr_qfs_id);

	configurable_->set_config(config_key_, q_qf_tuple);
}

void MeasuredQuantityProperty::on_value_changed(Glib::VariantBase g_var)
{
	Q_EMIT value_changed(QVariant(g_variant_get_string(g_var.gobj(), NULL)));
}

} // namespace datatypes
} // namespace devices
} // namespace sv
