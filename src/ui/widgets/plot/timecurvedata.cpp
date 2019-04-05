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

#include "timecurvedata.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::shared_ptr;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

TimeCurveData::TimeCurveData(shared_ptr<sv::data::AnalogTimeSignal> signal) :
	BaseCurveData(CurveType::TimeCurve),
	signal_(signal)
{
}

bool TimeCurveData::is_equal(const BaseCurveData *other) const
{
	const TimeCurveData *tcd = dynamic_cast<const TimeCurveData *>(other);
	if (tcd != nullptr) {
		return (signal_ == tcd->signal());
	}
	else {
		return false;
	}
}

QPointF TimeCurveData::sample(size_t i) const
{
	//signal_data_->lock();

	auto sample = signal_->get_sample(i, relative_time_);
	QPointF sample_point(sample.first, sample.second);

	//signal_data_->.unlock();

	return sample_point;
}

size_t TimeCurveData::size() const
{
	// TODO: Synchronize x/y sample data
	return signal_->get_sample_count();
}

QRectF TimeCurveData::boundingRect() const
{
	/*
	qWarning() << "TimeCurveData::boundingRect(): min time = "
		<< signal_->first_timestamp(relative_time_);
	qWarning() << "TimeCurveData::boundingRect(): max time = "
		<< signal_->last_timestamp(relative_time_);
	qWarning() << "TimeCurveData::boundingRect(): min value = "
		<< signal_->min_value();
	qWarning() << "TimeCurveData::boundingRect(): max value = "
		<< signal_->max_value();
	*/

	// top left, bottom right
	return QRectF(
		QPointF(signal_->first_timestamp(relative_time_), signal_->max_value()),
		QPointF(signal_->last_timestamp(relative_time_), signal_->min_value()));
}

QPointF TimeCurveData::closest_point(const QPointF &pos, double *dist) const
{
	(void)dist;
	const double x_value = pos.x();
	const int index_max = size() - 1;

	if (index_max < 0 || x_value >= sample(index_max).x())
		return QPointF(0, 0); // TODO

	size_t index_min = 0;
	size_t n = index_max;

	while (n > 0) {
		const size_t half = n >> 1;
		const size_t index_mid = index_min + half;

		if (x_value < sample(index_mid).x()) {
			n = half;
		}
		else {
			index_min = index_mid + 1;
			n -= half + 1;
		}
	}

	return sample(index_min);
}

QString TimeCurveData::name() const
{
	return signal_->display_name();
}

QString TimeCurveData::x_data_quantity() const
{
	return data::datautil::format_quantity(sv::data::Quantity::Time);
}

QString TimeCurveData::x_data_unit() const
{
	return data::datautil::format_unit(sv::data::Unit::Second);
}

QString TimeCurveData::x_data_title() const
{
	return QString("%1 [%2]").arg(x_data_quantity()).arg(x_data_unit());
}

QString TimeCurveData::y_data_quantity() const
{
	return signal_->quantity_name();
}

QString TimeCurveData::y_data_unit() const
{
	// Don't use signal_->unit_name(), so we can add AC/DC to axis label
	return data::datautil::format_unit(
		signal_->unit(), signal_->quantity_flags());
}

QString TimeCurveData::y_data_title() const
{
	return QString("%1 [%2]").arg(y_data_quantity()).arg(y_data_unit());
}

shared_ptr<sv::data::AnalogTimeSignal> TimeCurveData::signal() const
{
	return signal_;
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
