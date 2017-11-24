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

#include "curvedata.hpp"
#include "src/data/analogdata.hpp"

namespace sv {
namespace data {

CurveData::CurveData(shared_ptr<AnalogData> x_signal_data,
		shared_ptr<AnalogData> y_signal_data) :
	QwtSeriesData<QPointF>(),
	x_signal_data_(x_signal_data),
	y_signal_data_(y_signal_data)
{
}

QPointF CurveData::sample(size_t i) const
{
	//signal_data_->lock();

	QPointF sample(
		x_signal_data_->get_sample(i),
		y_signal_data_->get_sample(i));

	//signal_data_->.unlock();

	return sample;
}

size_t CurveData::size() const
{
	// TODO: Synchronize x/y sample data, so no compare is needed
	size_t x_size = x_signal_data_->get_sample_count();
	size_t y_size = y_signal_data_->get_sample_count();
	if (x_size < y_size)
		return x_size;
	else
		return y_size;
}

QRectF CurveData::boundingRect() const
{
	// top left (x, y), width, height
	return QRectF(
		x_signal_data_->min_value(), y_signal_data_->max_value(),
		x_signal_data_->max_value() - x_signal_data_->min_value(),
		y_signal_data_->max_value() - y_signal_data_->min_value());
}

} // namespace data
} // namespace sv
