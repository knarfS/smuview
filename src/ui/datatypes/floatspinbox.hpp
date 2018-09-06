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

#ifndef UI_DATATYPES_FLOATSPINBOX_HPP
#define UI_DATATYPES_FLOATSPINBOX_HPP

#include <memory>

#include <QDoubleSpinBox>
#include <QWidget>

#include "src/devices/deviceutil.hpp"
#include "src/data/datautil.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
class Configurable;
}

namespace ui {
namespace datatypes {

class FloatSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
	FloatSpinBox(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key, data::Unit unit, bool auto_commit,
		QWidget *parent = nullptr);

private:
	shared_ptr<devices::Configurable> configurable_;
	devices::ConfigKey config_key_;
	data::Unit unit_;
	bool auto_commit_;
	double min_;
	double max_;
	double step_;

	void setup_ui();
	void connect_signals();

public Q_SLOT:
	void change_value(double);

private Q_SLOTS:
	void on_value_changed(double);

};

} // namespace datatypes
} // namespace ui
} // namespece sv

#endif // UI_DATATYPES_FLOATSPINBOX_HPP

