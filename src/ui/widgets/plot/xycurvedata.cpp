/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include <QtMath>
#include <QPointF>
#include <QRectF>
#include <QSettings>
#include <QString>
#include <qwt_math.h>

#include "xycurvedata.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::dynamic_pointer_cast;
using std::lock_guard;
using std::make_shared;
using std::mutex;
using std::set;
using std::shared_ptr;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

XYCurveData::XYCurveData(shared_ptr<sv::data::AnalogTimeSignal> x_t_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_t_signal) :
	BaseCurveData(CurveDataType::XYDataCurve),
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
	if (xycd == nullptr)
		return false;

	return (x_t_signal_ == xycd->x_t_signal()) &&
		(y_t_signal_ == xycd->y_t_signal());
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
	if (num_samples == 0)
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

string XYCurveData::id_prefix() const
{
	return "xycurve";
}

sv::data::Quantity XYCurveData::x_quantity() const
{
	return x_t_signal_->quantity();
}

set<sv::data::QuantityFlag> XYCurveData::x_quantity_flags() const
{
	return x_t_signal_->quantity_flags();
}

sv::data::Unit XYCurveData::x_unit() const
{
	return x_t_signal_->unit();
}

QString XYCurveData::x_unit_str() const
{
	return data::datautil::format_unit(x_unit(), x_quantity_flags());
}


QString XYCurveData::x_title() const
{
	// Don't use only the unit, so we can add AC/DC to axis label.
	return QString("%1 [%2]").
		arg(data::datautil::format_quantity(x_quantity()), x_unit_str());
}

sv::data::Quantity XYCurveData::y_quantity() const
{
	return y_t_signal_->quantity();
}

set<sv::data::QuantityFlag> XYCurveData::y_quantity_flags() const
{
	return y_t_signal_->quantity_flags();
}

sv::data::Unit XYCurveData::y_unit() const
{
	return y_t_signal_->unit();
}

QString XYCurveData::y_unit_str() const
{
	return data::datautil::format_unit(y_unit(), y_quantity_flags());
}

QString XYCurveData::y_title() const
{
	// Don't use only the unit, so we can add AC/DC to axis label.
	return QString("%1 [%2]").
		arg(data::datautil::format_quantity(y_quantity()), y_unit_str());
}

shared_ptr<sv::data::AnalogTimeSignal> XYCurveData::x_t_signal() const
{
	return x_t_signal_;
}

shared_ptr<sv::data::AnalogTimeSignal> XYCurveData::y_t_signal() const
{
	return y_t_signal_;
}

void XYCurveData::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	SettingsManager::save_signal(x_t_signal_, settings, origin_device, "x_");
	SettingsManager::save_signal(y_t_signal_, settings, origin_device, "y_");
}

XYCurveData *XYCurveData::init_from_settings(
	Session &session, QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	auto x_t_signal = SettingsManager::restore_signal(
		session, settings, origin_device, "x_");
	auto y_t_signal = SettingsManager::restore_signal(
		session, settings, origin_device, "y_");
	if (!x_t_signal || !y_t_signal)
		return nullptr;

	return new XYCurveData(
		dynamic_pointer_cast<sv::data::AnalogTimeSignal>(x_t_signal),
		dynamic_pointer_cast<sv::data::AnalogTimeSignal>(y_t_signal));
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
