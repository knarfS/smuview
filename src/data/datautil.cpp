/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <map>
#include <set>

#include "datautil.hpp"

using std::map;
using std::set;

namespace sv {
namespace data {
namespace quantityutil {

Quantity get_quantity(const sigrok::Quantity *sr_quantity)
{
	if (sr_quantity_quantity_map.count(sr_quantity) > 0)
		return sr_quantity_quantity_map[sr_quantity];
	return Quantity::Unknown;
}

Quantity get_quantity(uint32_t sr_quantity)
{
	const sigrok::Quantity *sr_q = sigrok::Quantity::get(sr_quantity);
	return get_quantity(sr_q);
}

uint32_t get_sr_quantity_id(Quantity quantity)
{
	if (quantity_sr_quantity_map.count(quantity) > 0)
		return quantity_sr_quantity_map[quantity]->id();
	return 0;
}

QuantityFlag get_quantity_flag(const sigrok::QuantityFlag *sr_quantity_flag)
{
	if (sr_quantity_flag_quantity_flag_map.count(sr_quantity_flag) > 0)
		return sr_quantity_flag_quantity_flag_map[sr_quantity_flag];
	return QuantityFlag::Unknown;
}

uint64_t get_sr_quantity_flag_id(QuantityFlag quantity_flag)
{
	if (quantity_flag_sr_quantity_flag_map.count(quantity_flag) > 0)
		return quantity_flag_sr_quantity_flag_map[quantity_flag]->id();
	return 0;
}

set<QuantityFlag> get_quantity_flags(uint64_t sr_quantity_flags)
{
	set<data::QuantityFlag> quantity_flag_set;
	uint64_t mask = 1;
	for (uint i = 0; i < 32; i++, mask <<= 1) {
		if (!(sr_quantity_flags & mask))
			continue;

		const sigrok::QuantityFlag *sr_qf =
			sigrok::QuantityFlag::get(sr_quantity_flags & mask);
		quantity_flag_set.insert(get_quantity_flag(sr_qf));
	}

	return quantity_flag_set;
}

uint64_t get_sr_quantity_flags_id(set<QuantityFlag> quantity_flags)
{
	uint64_t sr_qfs_id = 0;
	for (QuantityFlag qunatity_flag : quantity_flags) {
		sr_qfs_id |= get_sr_quantity_flag_id(qunatity_flag);
	}
	return sr_qfs_id;
}

QString format_quantity(Quantity quantity)
{
	if (quantity_name_map.count(quantity) > 0)
		return quantity_name_map[quantity];
	return quantity_name_map[Quantity::Unknown];
}

QString format_quantity_flag(QuantityFlag quantity_flag)
{
	if (quantity_flag_name_map.count(quantity_flag) > 0)
		return quantity_flag_name_map[quantity_flag];
	return quantity_flag_name_map[QuantityFlag::Unknown];
}

QString format_quantity_flags(set<QuantityFlag> quantity_flags)
{
	QString qfs_str("");
	QString sep("");

	// Show AC/DC first
	if (quantity_flags.count(QuantityFlag::AC) > 0) {
		qfs_str.append(quantity_flag_name_map[QuantityFlag::AC]);
		sep = " ";
	}
	if (quantity_flags.count(QuantityFlag::DC) > 0) {
		qfs_str.append(sep);
		qfs_str.append(quantity_flag_name_map[QuantityFlag::DC]);
		sep = " ";
	}
	// 2nd is RMS
	if (quantity_flags.count(QuantityFlag::RMS) > 0) {
		qfs_str.append(sep);
		qfs_str.append(quantity_flag_name_map[QuantityFlag::RMS]);
		sep = " ";
	}

	// And now the rest of the flags
	for (auto qf : quantity_flags) {
		if (qf == QuantityFlag::AC || qf == QuantityFlag::DC ||
				qf == QuantityFlag::RMS)
			continue;

		if (quantity_flag_name_map.count(qf) == 0)
			continue;

		qfs_str.append(sep);
		qfs_str.append(quantity_flag_name_map[qf]);
		sep = " ";
	}

	return qfs_str;
}

} // namespace helper
} // namespace data
} // namespace sv
