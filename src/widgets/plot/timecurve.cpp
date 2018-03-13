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

#include "timecurve.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/datautil.hpp"
#include "src/widgets/plot/basecurve.hpp"

using std::shared_ptr;

namespace sv {
namespace widgets {
namespace plot {

TimeCurve::TimeCurve(shared_ptr<data::AnalogSignal> signal) :
	BaseCurve(),
	signal_(signal)
{
}

QPointF TimeCurve::sample(size_t i) const
{
	//signal_data_->lock();

	data::sample_t sample = signal_->get_sample(i, relative_time_);
	QPointF sample_point(sample.first, sample.second);

	//signal_data_->.unlock();

	return sample_point;
}

size_t TimeCurve::size() const
{
	// TODO: Synchronize x/y sample data
	return signal_->get_sample_count();
}

QRectF TimeCurve::boundingRect() const
{
	/*
	qWarning() << "TimeCurve::boundingRect(): min time = "
		<< signal_->first_timestamp(relative_time_);
	qWarning() << "TimeCurve::boundingRect(): max time = "
		<< signal_->last_timestamp(relative_time_);
	qWarning() << "TimeCurve::boundingRect(): min value = "
		<< signal_->min_value();
	qWarning() << "TimeCurve::boundingRect(): max value = "
		<< signal_->max_value();
	*/

	// top left, bottom right
	return QRectF(
		QPointF(signal_->first_timestamp(relative_time_), signal_->max_value()),
		QPointF(signal_->last_timestamp(relative_time_), signal_->min_value()));
}

QString TimeCurve::name() const
{
	return signal_->name();
}

QString TimeCurve::x_data_quantity() const
{
	return data::quantityutil::format_quantity(data::Quantity::Time);
}

QString TimeCurve::x_data_unit() const
{
	return data::quantityutil::format_unit(data::Unit::Second);
}

QString TimeCurve::x_data_title() const
{
	return QString("%1 [%2]").arg(x_data_quantity()).arg(x_data_unit());
}

QString TimeCurve::y_data_quantity() const
{
	return signal_->quantity_name();
}

QString TimeCurve::y_data_unit() const
{
	return signal_->unit_name();
}

QString TimeCurve::y_data_title() const
{
	return QString("%1 [%2]").arg(y_data_quantity()).arg(y_data_unit());
}

} // namespace plot
} // namespace widgets
} // namespace sv
