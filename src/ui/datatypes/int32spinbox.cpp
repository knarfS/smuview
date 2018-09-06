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

#include "int32spinbox.hpp"
#include "src/util.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace ui {
namespace datatypes {

Int32SpinBox::Int32SpinBox(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key, data::Unit unit, bool auto_commit,
		QWidget *parent) :
	QSpinBox(parent),
	configurable_(configurable),
	config_key_(config_key),
	unit_(unit),
	auto_commit_(auto_commit)
{
	setup_ui();
	connect_signals();
}

void Int32SpinBox::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	if (configurable_->has_list_config(config_key_)) {
		configurable_->list_config_min_max_step(config_key_, min_, max_, step_);
		this->setRange(min_, max_);
		this->setSingleStep(step_);
	}
	if (unit_ != data::Unit::Unknown && unit_ != data::Unit::Unitless) {
		this->setSuffix(QString(" %1").arg(data::datautil::format_unit(unit_)));
	}
	this->setDisabled(!configurable_->has_set_config(config_key_));
}

void Int32SpinBox::connect_signals()
{
	if (auto_commit_ && configurable_->has_set_config(config_key_))
		connect(this, SIGNAL(valueChanged(int32_t)),
			this, SLOT(on_value_changed(int32_t)));
}

void Int32SpinBox::change_value(int32_t value)
{
	this->setValue(value);
}

void Int32SpinBox::on_value_changed(int32_t value)
{
	configurable_->set_config<int32_t>(config_key_, value);
}

} // namespace datatypes
} // namespace ui
} // namespace sv
