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

#include <memory>

#include <QPointF>
#include <QRectF>
#include <QString>

#include "xycurvedata.hpp"
#include "src/data/analogsignal.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::shared_ptr;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

XYCurveData::XYCurveData(shared_ptr<sv::data::AnalogSignal> x_signal,
		shared_ptr<sv::data::AnalogSignal> y_signal) :
	BaseCurveData(CurveType::XYCurve),
	x_signal_(x_signal),
	y_signal_(y_signal)
{
}

QPointF XYCurveData::sample(size_t i) const
{
	//signal_data_->lock();

	// TODO: synchronize timestamps between signals, that are not
	//       from the same frame
	data::sample_t x_sample = x_signal_->get_sample(i, relative_time_);
	data::sample_t y_sample = y_signal_->get_sample(i, relative_time_);

	QPointF sample_point(x_sample.second, y_sample.second);

	//signal_data_->.unlock();

	return sample_point;
}

size_t XYCurveData::size() const
{
	// TODO: Synchronize x/y sample data
	return x_signal_->get_sample_count();
}

QRectF XYCurveData::boundingRect() const
{
	// top left, bottom right
	return QRectF(
		QPointF(x_signal_->min_value(), y_signal_->max_value()),
		QPointF(x_signal_->max_value(), y_signal_->min_value()));
}

QPointF XYCurveData::closest_point(const QPointF &pos, double *dist) const
{
	const size_t num_samples = size();
	if (num_samples <= 0)
		return QPointF(0, 0); // TODO

	size_t index = -1;
	double dmin = 1.0e10;

	for (size_t i=0; i < num_samples; i++) {
		const QPointF s = sample(i);
		const double cx = s.x() - pos.x();
		const double cy = s.y() - pos.y();
		const double d = qwtSqr(cx) + qwtSqr(cy);
		if (d < dmin) {
			index = i;
			dmin = d;
		}
	}
	if (dist)
		*dist = qSqrt(dmin);

	return sample(index);
}

QString XYCurveData::name() const
{
	return y_signal_->name().append(" -> ").append(x_signal_->name());
}

QString XYCurveData::x_data_quantity() const
{
	return x_signal_->quantity_name();
}

QString XYCurveData::x_data_unit() const
{
	return x_signal_->unit_name();
}

QString XYCurveData::x_data_title() const
{
	return QString("%1 [%2]").arg(x_data_quantity()).arg(x_data_unit());
}

QString XYCurveData::y_data_quantity() const
{
	return y_signal_->quantity_name();
}

QString XYCurveData::y_data_unit() const
{
	return y_signal_->unit_name();
}

QString XYCurveData::y_data_title() const
{
	return QString("%1 [%2]").arg(y_data_quantity()).arg(y_data_unit());
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
