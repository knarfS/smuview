/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DATA_TIMECURVE_HPP
#define DATA_TIMECURVE_HPP

#include <memory>
#include <vector>

#include <QPointer>
#include <qwt_series_data.h>

#include "src/data/basecurve.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {
namespace data {

class AnalogSignal;

class TimeCurve : public BaseCurve
{

public:
	TimeCurve(shared_ptr<AnalogSignal> signal);

	QPointF sample( size_t i ) const;
	size_t size() const;
	QRectF boundingRect() const;

	void set_relative_time(bool is_relative_time);
	bool is_relative_time() const;
	bool is_initialized() const;
	QString x_data_quantity() const;
	QString x_data_unit() const;
	QString x_data_title() const;
	QString y_data_quantity() const;
	QString y_data_unit() const;
	QString y_data_title() const;

private:
	shared_ptr<AnalogSignal> analog_signal_;
	double signal_start_timestamp_;
	bool relative_time_;

};

} // namespace data
} // namespace sv

#endif // DATA_TIMECURVE_HPP
