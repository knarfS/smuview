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

#include <chrono>
#include <thread>

#include <QDebug>

#include "stepblock.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/properties/baseproperty.hpp"
#include "src/processing/processor.hpp"

using std::shared_ptr;

namespace sv {
namespace processing {

StepBlock::StepBlock(const shared_ptr<Processor> processor) :
		BaseBlock(processor),
	step_cnt_(0)
{
}

void StepBlock::init()
{
	// Create a linear step sequence 0..10 with 0.1 step size every 100ms
	step_cnt_ = 100;

	for (ulong i=0; i<step_cnt_; i++) {
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
	//QVariant::Type type = start_value_.type();
	QVariant act_value = start_value_;
	while (act_value <= end_value_ && processor_->is_running()) {
		qWarning() << "StepBlock: value = " << act_value <<
			", delay = " << delay_ms_ << " ms.";

		property_->change_value(act_value);
		double d_val = act_value.toDouble() + step_size_.toDouble();
		act_value = QVariant(d_val);
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_));
	}
}

shared_ptr<devices::properties::BaseProperty> StepBlock::property() const
{
	return property_;
}

void StepBlock::set_property(
	shared_ptr<devices::properties::BaseProperty> property)
{
	property_ = property;
}

QVariant StepBlock::start_value() const
{
	return start_value_;
}

void StepBlock::set_start_value(QVariant start_value)
{
	start_value_ = start_value;
}

QVariant StepBlock::end_value() const
{
	return end_value_;
}

void StepBlock::set_end_value(QVariant end_value)
{
	end_value_ = end_value;
}

QVariant StepBlock::step_size() const
{
	return start_value_;
}

void StepBlock::set_step_size(QVariant step_size)
{
	step_size_ = step_size;
}

uint StepBlock::delay_ms() const
{
	return delay_ms_;
}

void StepBlock::set_delay_ms(uint delay_ms)
{
	delay_ms_ = delay_ms;
}

} // namespace processing
} // namespace sv
