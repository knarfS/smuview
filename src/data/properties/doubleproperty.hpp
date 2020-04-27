/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_PROPERTIES_DOUBLEPROPERTY_HPP
#define DATA_PROPERTIES_DOUBLEPROPERTY_HPP

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

class DoubleProperty : public BaseProperty
{
	Q_OBJECT

public:
	DoubleProperty(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key);

public:
	QVariant value() const override;
	double double_value() const;
	QString to_string(double value) const;
	QString to_string(const QVariant qvar) const override;
	QString to_string() const override;
	double min() const;
	double max() const;
	double step() const;
	uint digits() const;
	uint decimal_places() const;

private:
	double min_;
	double max_;
	double step_;
	uint digits_;
	uint decimal_places_;

public Q_SLOTS:
	bool list_config() override;
	void change_value(const QVariant qvar) override;
	void on_value_changed(Glib::VariantBase gvar) override;

};

} // namespace properties
} // namespace data
} // namespece sv

#endif // DATA_PROPERTIES_DOUBLEPROPERTY_HPP
