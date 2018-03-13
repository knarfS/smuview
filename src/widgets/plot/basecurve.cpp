/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <QColor>
#include <qwt_series_data.h>

#include "basecurve.hpp"

namespace sv {
namespace widgets {
namespace plot {

BaseCurve::BaseCurve() :
	QwtSeriesData<QPointF>(),
	relative_time_(true)
{
}

QColor BaseCurve::color() const
{
	// TODO
	if (y_data_unit() == "V")
		return Qt::red;
	else if (y_data_unit() == "A")
		return Qt::green;
	else
		return Qt::blue;

	//return color_;
}

void BaseCurve::set_relative_time(bool is_relative_time)
{
	relative_time_ = is_relative_time;
}

bool BaseCurve::is_relative_time() const
{
	return relative_time_;
}

} // namespace plot
} // namespace widgets
} // namespace sv
