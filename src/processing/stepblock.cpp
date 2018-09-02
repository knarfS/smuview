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

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>

#include "stepblock.hpp"
#include "src/devices/configurable.hpp"

namespace sv {
namespace processing {

StepBlock::StepBlock() : BaseBlock(),
	step_cnt_(0)
{
}

void StepBlock::init()
{
	// Create a linear step sequence 0..10 with 0.1 step size every 100ms
	long delay = 100;
	step_cnt_ = 100;

	for (long i=0; i<step_cnt_; i++) {
		values_.push_back((double)0.1 * i);
		delays_.push_back(delay);

		qWarning() << "StepBlock init: value = " << values_.at(i) <<
			", delay = " << delays_.at(i) << " ms.";
	}
}

void StepBlock::run()
{
	//assert(configurable_);
	//assert(config_key_);

	for (long i=0; i<step_cnt_; i++) {
		qWarning() << "StepBlock: value = " << values_.at(i) <<
			", delay = " << delays_.at(i) << " ms.";

		configurable_->set_amplitude(values_.at(i));
		configurable_->amplitude_changed(values_.at(i));
		//configurable_->set_config(config_key_, values_.at(i));
		std::this_thread::sleep_for(std::chrono::milliseconds(delays_.at(i)));
	}
}

void StepBlock::set_configurable(shared_ptr<devices::Configurable> configurable)
{
	configurable_ = configurable;
}

void StepBlock::set_config_key(const sigrok::ConfigKey *key)
{
	config_key_ = key;
}

} // namespace processing
} // namespace sv
