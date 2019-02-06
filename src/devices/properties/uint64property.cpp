/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QString>
#include <QVariant>

#include "uint64property.hpp"
#include "src/devices/configurable.hpp"

using std::string;

namespace sv {
namespace devices {
namespace properties {

UInt64Property::UInt64Property(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key) :
	BaseProperty(configurable, config_key),
	min_(std::numeric_limits<uint64_t>::lowest()),
	max_(std::numeric_limits<uint64_t>::max()),
	step_(1)
{
	if (is_listable_)
		list_config();

	/*
	if (unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless) {
		this->setSuffix(QString(" %1").arg(data::datautil::format_unit(unit_)));
	}
	*/
}

QVariant UInt64Property::value() const
{
	return QVariant((qulonglong)uint64_value());
}

uint64_t UInt64Property::uint64_value() const
{
	return configurable_->get_config<uint64_t>(config_key_);
}

QString UInt64Property::to_string() const
{
	return QString("%1").arg(uint64_value());
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

vector<uint64_t> UInt64Property::values() const
{
	return values_;
}

bool UInt64Property::list_config()
{
	Glib::VariantContainerBase gvar;
	if (!configurable_->list_config(config_key_, gvar))
		return false;

	if (config_key_ == ConfigKey::Samplerate) {
		GVariant *gvar_list;
		const uint64_t *elements = nullptr;
		gsize num_elements;
		if ((gvar_list = g_variant_lookup_value(gvar.gobj(),
				"samplerate-steps", G_VARIANT_TYPE("at")))) {
			elements = (const uint64_t *)g_variant_get_fixed_array(
				gvar_list, &num_elements, sizeof(uint64_t));
			min_ = elements[0];
			max_ = elements[1];
			step_ = elements[2];
			g_variant_unref(gvar_list);
		}
		else if ((gvar_list = g_variant_lookup_value(gvar.gobj(),
				"samplerates", G_VARIANT_TYPE("at")))) {
			elements = (const uint64_t *)g_variant_get_fixed_array(
				gvar_list, &num_elements, sizeof(uint64_t));
			for (size_t i=0; i<num_elements; i++) {
				values_.push_back(elements[i]);
			}
			g_variant_unref(gvar_list);
		}
		else {
			return false;
		}
	}
	else {
		Glib::VariantIter iter(gvar);
		iter.next_value(gvar);
		min_ = Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(gvar).get();
		iter.next_value(gvar);
		max_ = Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(gvar).get();
		iter.next_value(gvar);
		step_ = Glib::VariantBase::cast_dynamic<Glib::Variant<uint64_t>>(gvar).get();
	}

	return true;
}

void UInt64Property::change_value(const QVariant qvar)
{
	configurable_->set_config(config_key_, (uint64_t)qvar.toULongLong());
	Q_EMIT value_changed(qvar);
}

void UInt64Property::on_value_changed(Glib::VariantBase g_var)
{
	Q_EMIT value_changed(QVariant(
		(qulonglong)g_variant_get_uint64(g_var.gobj())));
}

} // namespace datatypes
} // namespace devices
} // namespace sv
