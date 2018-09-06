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

#include <QDebug>

#include "floatspinbox.hpp"
#include "src/util.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace ui {
namespace datatypes {

FloatSpinBox::FloatSpinBox(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key, data::Unit unit, bool auto_commit,
		QWidget *parent) :
	QDoubleSpinBox(parent),
	configurable_(configurable),
	config_key_(config_key),
	unit_(unit),
	auto_commit_(auto_commit)
{
	setup_ui();
	connect_signals();
}

void FloatSpinBox::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (configurable_->has_list_config(config_key_)) {
		configurable_->list_config_min_max_step(config_key_, min_, max_, step_);
		this->setRange(min_, max_);
		this->setSingleStep(step_);
		this->setDecimals(util::get_decimal_places(step_));
	}
	else {
		this->setDecimals(3);
	}
	if (unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless) {
		this->setSuffix(QString(" %1").arg(data::datautil::format_unit(unit_)));
	}
	this->setDisabled(!configurable_->has_set_config(config_key_));
}

void FloatSpinBox::connect_signals()
{
	if (auto_commit_ && configurable_->has_set_config(config_key_))
		connect(this, SIGNAL(valueChanged(double)),
			this, SLOT(on_value_changed(double)));
}

void FloatSpinBox::change_value(double value)
{
	this->setValue(value);
}

void FloatSpinBox::on_value_changed(double value)
{
	configurable_->set_config<double>(config_key_, value);
}

} // namespace datatypes
} // namespace ui
} // namespace sv
