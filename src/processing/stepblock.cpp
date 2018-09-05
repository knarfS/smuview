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

#include <cassert>
#include <chrono>
#include <thread>

#include <QDebug>

#include "stepblock.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"

namespace sv {
namespace processing {

StepBlock::StepBlock() : BaseBlock(),
	step_cnt_(0)
{
}

void StepBlock::init()
{
	// Create a linear step sequence 0..10 with 0.1 step size every 100ms
	step_cnt_ = 100;

	for (long i=0; i<step_cnt_; i++) {
		/*
		values_.push_back((double)0.1 * i);
		delays_.push_back(delay);

		qWarning() << "StepBlock init: value = " << values_.at(i) <<
			", delay = " << delays_.at(i) << " ms.";
		*/
	}
}

void StepBlock::run()
{
	assert(configurable_);
	assert(config_key_);

	double act_value = start_value_;
	while (act_value <= end_value_) {
		qWarning() << "StepBlock: value = " << act_value <<
			", delay = " << delay_ms_ << " ms.";

		configurable_->set_config(config_key_, act_value);
		configurable_->config_changed(config_key_, act_value);
		act_value += step_size_;
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_));
	}
}

void StepBlock::set_configurable(shared_ptr<devices::Configurable> configurable)
{
	configurable_ = configurable;
}

void StepBlock::set_config_key(devices::ConfigKey key)
{
	config_key_ = key;
}

void StepBlock::set_start_value(double start_value)
{
	start_value_ = start_value;
}

void StepBlock::set_end_value(double end_value)
{
	end_value_ = end_value;
}

void StepBlock::set_step_size(double step_size)
{
	step_size_ = step_size;
}

void StepBlock::set_delay_ms(int delay_ms)
{
	delay_ms_ = delay_ms;
}

} // namespace processing
} // namespace sv
