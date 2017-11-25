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

#include "basecurve.hpp"
#include "src/data/analogdata.hpp"

namespace sv {
namespace data {

BaseCurve::BaseCurve(shared_ptr<AnalogData> x_signal_data,
		shared_ptr<AnalogData> y_signal_data) :
	QwtSeriesData<QPointF>(),
	x_signal_data_(x_signal_data),
	y_signal_data_(y_signal_data)
{
}

QPointF BaseCurve::sample(size_t i) const
{
	//signal_data_->lock();

	QPointF sample(
		x_signal_data_->get_sample(i),
		y_signal_data_->get_sample(i));

	//signal_data_->.unlock();

	return sample;
}

size_t BaseCurve::size() const
{
	// TODO: Synchronize x/y sample data, so no compare is needed
	size_t x_size = x_signal_data_->get_sample_count();
	size_t y_size = y_signal_data_->get_sample_count();
	if (x_size < y_size)
		return x_size;
	else
		return y_size;
}

QRectF BaseCurve::boundingRect() const
{
	// top left (x, y), width, height
	return QRectF(
		x_signal_data_->min_value(), y_signal_data_->max_value(),
		x_signal_data_->max_value() - x_signal_data_->min_value(),
		y_signal_data_->max_value() - y_signal_data_->min_value());
}

QString BaseCurve::x_signal_quantity() const
{
	return x_signal_data_->quantity();
}

QString BaseCurve::x_signal_unit() const
{
	return x_signal_data_->unit();
}

QString BaseCurve::x_signal_title() const
{
	return QString("%1 [%2]").
		arg(x_signal_data_->quantity()).arg(x_signal_data_->unit());
}

QString BaseCurve::y_signal_quantity() const
{
	return y_signal_data_->quantity();
}

QString BaseCurve::y_signal_unit() const
{
	return y_signal_data_->unit();
}

QString BaseCurve::y_signal_title() const
{
	return QString("%1 [%2]").
		arg(y_signal_data_->quantity()).arg(y_signal_data_->unit());
}

} // namespace data
} // namespace sv
