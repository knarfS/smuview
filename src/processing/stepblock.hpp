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

#ifndef PROCESSING_STEPBLOCK_HPP
#define PROCESSING_STEPBLOCK_HPP

#include <memory>
#include <vector>

#include <QObject>

#include "src/devices/deviceutil.hpp"
#include "src/processing/baseblock.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {

namespace devices {
class Configurable;
}

namespace processing {

class StepBlock :
	public BaseBlock
{
	Q_OBJECT

public:
	StepBlock();

public:
	void init();
	void run();

	void set_configurable(shared_ptr<devices::Configurable>);
	void set_config_key(devices::ConfigKey);
	void set_start_value(double start_value);
	void set_end_value(double end_value);
	void set_step_size(double step_size);
	void set_delay_ms(int delay_ms);

private:
	shared_ptr<devices::Configurable> configurable_;
	devices::ConfigKey config_key_;

	double start_value_;
	double end_value_;
	double step_size_;
	int delay_ms_;
	long step_cnt_;

	/*
	vector<double> values_;
	vector<long> delays_;
	*/

};

} // namespace processing
} // namespace sv

#endif // PROCESSING_STEPBLOCK_HPP
