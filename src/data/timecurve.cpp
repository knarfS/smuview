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

#include "timecurve.hpp"
#include "src/util.hpp"
#include "src/data/analogsignal.hpp"

namespace sv {
namespace data {

TimeCurve::TimeCurve(shared_ptr<AnalogSignal> signal) :
	BaseCurve(),
	signal_(signal),
	relative_time_(true)
{
}

QPointF TimeCurve::sample(size_t i) const
{
	//signal_data_->lock();

	sample_t sample = signal_->get_sample(i, relative_time_);
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
	// top left, bottom right
	return QRectF(
		QPointF(signal_->first_timestamp(relative_time_), signal_->max_value()),
		QPointF(signal_->last_timestamp(relative_time_), signal_->min_value()));
}

void TimeCurve::set_relative_time(bool is_relative_time)
{
	relative_time_ = is_relative_time;
}

bool TimeCurve::is_relative_time() const
{
	return relative_time_;
}

QString TimeCurve::name() const
{
	return signal_->name();
}

QString TimeCurve::x_data_quantity() const
{
	return util::format_sr_quantity(sigrok::Quantity::TIME);
}

QString TimeCurve::x_data_unit() const
{
	return util::format_sr_unit(sigrok::Unit::SECOND);
}

QString TimeCurve::x_data_title() const
{
	return QString("%1 [%2]").arg(x_data_quantity()).arg(x_data_unit());
}

QString TimeCurve::y_data_quantity() const
{
	return signal_->quantity();
}

QString TimeCurve::y_data_unit() const
{
	return signal_->unit();
}

QString TimeCurve::y_data_title() const
{
	return QString("%1 [%2]").arg(y_data_quantity()).arg(y_data_unit());
}

} // namespace data
} // namespace sv
