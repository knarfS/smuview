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

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDateTime>

#include "xycurve.hpp"
#include "src/util.hpp"
#include "src/data/analogsignal.hpp"

namespace sv {
namespace data {

XYCurve::XYCurve(shared_ptr<AnalogSignal> x_signal,
		shared_ptr<AnalogSignal> y_signal) :
	BaseCurve(),
	x_signal_(x_signal),
	y_signal_(y_signal),
	relative_time_(true)
{
	//TODO
	//signal_start_timestamp_ = signal->signal_start_timestamp();
	signal_start_timestamp_ =
		QDateTime::currentMSecsSinceEpoch() / (double)1000;
	//qWarning() << "XYCurve::XYCurve(): signal_start_timestamp_ = " <<
	//	signal_start_timestamp_;
}

QPointF XYCurve::sample(size_t i) const
{
	//signal_data_->lock();

	// TODO: synchronize timestamps between signals, that are not
	//       from the same frame
	sample_t x_sample = x_signal_->get_sample(i);
	sample_t y_sample = y_signal_->get_sample(i);

	QPointF sample_point(x_sample.second, y_sample.second);

	//signal_data_->.unlock();

	return sample_point;
}

size_t XYCurve::size() const
{
	// TODO: Synchronize x/y sample data
	return x_signal_->get_sample_count();
}

QRectF XYCurve::boundingRect() const
{
	// top left, bottom right
	return QRectF(
		QPointF(x_signal_->min_value(), y_signal_->max_value()),
		QPointF(x_signal_->max_value(), y_signal_->min_value()));
}

void XYCurve::set_relative_time(bool is_relative_time)
{
	relative_time_ = is_relative_time;
}

bool XYCurve::is_relative_time() const
{
	return relative_time_;
}

QString XYCurve::x_data_quantity() const
{
	return x_signal_->quantity();
}

QString XYCurve::x_data_unit() const
{
	return x_signal_->unit();
}

QString XYCurve::x_data_title() const
{
	return QString("%1 [%2]").arg(x_data_quantity()).arg(x_data_unit());
}

QString XYCurve::y_data_quantity() const
{
	return y_signal_->quantity();
}

QString XYCurve::y_data_unit() const
{
	return y_signal_->unit();
}

QString XYCurve::y_data_title() const
{
	return QString("%1 [%2]").arg(y_data_quantity()).arg(y_data_unit());
}

} // namespace data
} // namespace sv
