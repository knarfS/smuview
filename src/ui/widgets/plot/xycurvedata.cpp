/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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
#include <mutex>
#include <vector>

#include <QPointF>
#include <QRectF>
#include <QString>

#include "xycurvedata.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::lock_guard;
using std::make_shared;
using std::mutex;
using std::shared_ptr;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

XYCurveData::XYCurveData(shared_ptr<sv::data::AnalogTimeSignal> x_t_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_t_signal) :
	BaseCurveData(CurveType::XYCurve),
	x_t_signal_(x_t_signal),
	y_t_signal_(y_t_signal),
	x_t_signal_pos_(0),
	y_t_signal_pos_(0)
{
	x_data_ = make_shared<vector<double>>();
	y_data_ = make_shared<vector<double>>();

	// Prefill data vectors
	this->on_sample_appended();

	connect(x_t_signal_.get(), SIGNAL(sample_appended()),
		this, SLOT(on_sample_appended()));
	connect(y_t_signal_.get(), SIGNAL(sample_appended()),
		this, SLOT(on_sample_appended()));
}

bool XYCurveData::is_equal(const BaseCurveData *other) const
{
	const XYCurveData *xycd = dynamic_cast<const XYCurveData *>(other);
	if (xycd != nullptr) {
		return (x_t_signal_ == xycd->x_t_signal()) &&
			(y_t_signal_ == xycd->y_t_signal());
	}
	else {
		return false;
	}
}

QPointF XYCurveData::sample(size_t i) const
{
	QPointF sample_point(x_data_->at(i), y_data_->at(i));
	return sample_point;
}

size_t XYCurveData::size() const
{
	return x_data_->size();
}

QRectF XYCurveData::boundingRect() const
{
	// top left, bottom right
	return QRectF(
		QPointF(x_t_signal_->min_value(), y_t_signal_->max_value()),
		QPointF(x_t_signal_->max_value(), y_t_signal_->min_value()));
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
	return y_t_signal_->display_name().append(" -> ").
		append(x_t_signal_->display_name());
}

QString XYCurveData::x_data_quantity() const
{
	return x_t_signal_->quantity_name();
}

QString XYCurveData::x_data_unit() const
{
	// Don't use x_t_signal_->unit_name(), so we can add AC/DC to axis label
	return data::datautil::format_unit(
		x_t_signal_->unit(), x_t_signal_->quantity_flags());
}

QString XYCurveData::x_data_title() const
{
	return QString("%1 [%2]").arg(x_data_quantity()).arg(x_data_unit());
}

QString XYCurveData::y_data_quantity() const
{
	return y_t_signal_->quantity_name();
}

QString XYCurveData::y_data_unit() const
{
	// Don't use y_t_signal_->unit_name(), so we can add AC/DC to axis label
	return data::datautil::format_unit(
		y_t_signal_->unit(), y_t_signal_->quantity_flags());
}

QString XYCurveData::y_data_title() const
{
	return QString("%1 [%2]").arg(y_data_quantity()).arg(y_data_unit());
}

shared_ptr<sv::data::AnalogTimeSignal> XYCurveData::x_t_signal() const
{
	return x_t_signal_;
}

shared_ptr<sv::data::AnalogTimeSignal> XYCurveData::y_t_signal() const
{
	return y_t_signal_;
}

void XYCurveData::on_sample_appended()
{
	lock_guard<mutex> lock(sample_append_mutex_);

	shared_ptr<vector<double>> time = make_shared<vector<double>>();
	sv::data::AnalogTimeSignal::combine_signals(
		x_t_signal_, x_t_signal_pos_,
		y_t_signal_, y_t_signal_pos_,
		time, x_data_, y_data_);
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
