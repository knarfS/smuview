/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <limits>
#include <string>
#include <vector>

#include <QDebug>
#include <QString>
#include <QVariant>

#include "uint64property.hpp"
#include "src/util.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"

using std::string;
using std::vector;

namespace sv {
namespace data {
namespace properties {

UInt64Property::UInt64Property(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key),
	min_(std::numeric_limits<uint64_t>::lowest()),
	max_(std::numeric_limits<uint64_t>::max()),
	step_(1)
{
	if (is_listable_)
		UInt64Property::list_config();
}

QVariant UInt64Property::value() const
{
	return QVariant((qulonglong)uint64_value());
}

uint64_t UInt64Property::uint64_value() const
{
	uint64_t uint64_value;
	configurable_->get_config<uint64_t>(config_key_, uint64_value);
	return uint64_value;
}

QString UInt64Property::to_string(uint64_t value) const
{
	// TODO: calc digits+ decimal_places from min/max/step
	QString str;
	QString si_prefix;
	util::format_value_si(static_cast<double>(value), -1, 1, str, si_prefix);
	if (!si_prefix.isEmpty() ||
			(unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless))
		str.append(" ").append(si_prefix).append(datautil::format_unit(unit_));

	return str;
}

QString UInt64Property::to_string(const QVariant &qvar) const
{
	return this->to_string(qvar.toULongLong());
}

QString UInt64Property::to_string() const
{
	return this->to_string(uint64_value());
}

uint64_t UInt64Property::min() const
{
	return min_;
}

uint64_t UInt64Property::max() const
{
	return max_;
}

uint64_t UInt64Property::step() const
{
	return step_;
}

vector<uint64_t> UInt64Property::list_values() const
{
	return values_list_;
}

bool UInt64Property::list_config()
{
	values_list_.clear();

	Glib::VariantContainerBase gvar;
	if (!configurable_->list_config(config_key_, gvar))
		return false;

	if (config_key_ == devices::ConfigKey::Samplerate) {
		GVariant *gvar_list;
		const uint64_t *elements = nullptr;
		gsize num_elements;
		if ((gvar_list = g_variant_lookup_value(gvar.gobj(),
				"samplerate-steps", G_VARIANT_TYPE("at")))) {
			elements = static_cast<const uint64_t *>(g_variant_get_fixed_array(
				gvar_list, &num_elements, sizeof(uint64_t)));
			min_ = elements[0];
			max_ = elements[1];
			step_ = elements[2];
			g_variant_unref(gvar_list);
		}
		else if ((gvar_list = g_variant_lookup_value(gvar.gobj(),
				"samplerates", G_VARIANT_TYPE("at")))) {
			elements = static_cast<const uint64_t *>(g_variant_get_fixed_array(
				gvar_list, &num_elements, sizeof(uint64_t)));
			for (size_t i=0; i<num_elements; i++) {
				values_list_.push_back(elements[i]);
			}
			g_variant_unref(gvar_list);
		}
		else {
			return false;
		}
	}
	else if (config_key_ == devices::ConfigKey::SampleInterval) {
		// TODO: *data = std_gvar_tuple_array(ARRAY_AND_SIZE(kecheng_kc_330b_sample_intervals));
		Glib::VariantIter iter(gvar);
		while (iter.next_value (gvar)) {
			uint64_t low = Glib::VariantBase::cast_dynamic
				<Glib::Variant<uint64_t>>(gvar.get_child(0)).get();
			uint64_t high = Glib::VariantBase::cast_dynamic
				<Glib::Variant<uint64_t>>(gvar.get_child(1)).get();

			(void)low;
			(void)high;
			//values_list_.push_back(make_pair(low, high));
		}
	}
	else {
		Glib::VariantIter iter(gvar);
		while (iter.next_value (gvar)) {
			values_list_.push_back(
				Glib::VariantBase::cast_dynamic<Glib::Variant<guint64>>(gvar).get());
		}
	}

	Q_EMIT list_changed();

	return true;
}

void UInt64Property::change_value(const QVariant &qvar)
{
	/*
	 * TODO: This is a dirty hack to limit the sample rate of the (demo) device
	 *       to 20 kSamples/s to prevent memory overflow.
	 *       To fix this hack, a proper memory management has to be implemented!
	 */
	QVariant new_qvar = qvar;
	if (config_key_ == devices::ConfigKey::Samplerate) {
		uint64_t sample_rate = (uint64_t)new_qvar.toULongLong();
		if (sample_rate > 20000)
			new_qvar.setValue((qulonglong)20000);
	}

	configurable_->set_config(config_key_, (uint64_t)new_qvar.toULongLong());
	Q_EMIT value_changed(new_qvar);
}

void UInt64Property::on_value_changed(Glib::VariantBase gvar)
{
	Q_EMIT value_changed(QVariant(
		(qulonglong)g_variant_get_uint64(gvar.gobj())));
}

} // namespace properties
} // namespace data
} // namespace sv
