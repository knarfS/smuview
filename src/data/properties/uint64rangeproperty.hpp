/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_PROPERTIES_UINT64RANGEPROPERTY_HPP
#define DATA_PROPERTIES_UINT64RANGEPROPERTY_HPP

#include <memory>

#include <glib.h>

#include <QObject>
#include <QString>
#include <QVariant>

#include "src/data/properties/baseproperty.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/deviceutil.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
class Configurable;
}

namespace data {
namespace properties {

class UInt64RangeProperty : public BaseProperty
{
	Q_OBJECT

public:
	UInt64RangeProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key);

public:
	QVariant value() const override;
	data::uint64_range_t uint64_range_value() const;
	QString to_string(data::uint64_range_t value) const;
	QString to_string(const QVariant qvar) const override;
	QString to_string() const override;
	vector<data::uint64_range_t> list_values() const;

private:
	vector<data::uint64_range_t> values_list_;

public Q_SLOTS:
	bool list_config() override;
	void change_value(const QVariant qvar) override;
	void on_value_changed(Glib::VariantBase gvar) override;

};

} // namespace properties
} // namespace data
} // namespece sv

#endif // DATA_PROPERTIES_UINT64RANGEPROPERTY_HPP
