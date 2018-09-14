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
#include <QVariant>

#include "src/devices/deviceutil.hpp"
#include "src/processing/baseblock.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {

namespace devices {
namespace properties {
class BaseProperty;
}
}

namespace processing {

class StepBlock :
	public BaseBlock
{
	Q_OBJECT

public:
	StepBlock(const shared_ptr<Processor> processor);

public:
	void init();
	void run();

	shared_ptr<devices::properties::BaseProperty> property() const;
	void set_property(shared_ptr<devices::properties::BaseProperty>);
	QVariant start_value() const;
	void set_start_value(QVariant);
	QVariant end_value() const;
	void set_end_value(QVariant);
	QVariant step_size() const;
	void set_step_size(QVariant);
	uint delay_ms() const;
	void set_delay_ms(uint);

private:
	shared_ptr<devices::properties::BaseProperty> property_;
	QVariant start_value_;
	QVariant end_value_;
	QVariant step_size_;
	uint delay_ms_;
	ulong step_cnt_;

	/*
	vector<double> values_;
	vector<long> delays_;
	*/

};

} // namespace processing
} // namespace sv

#endif // PROCESSING_STEPBLOCK_HPP
