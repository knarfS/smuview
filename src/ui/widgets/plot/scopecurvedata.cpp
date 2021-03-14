/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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
#include <set>

#include <QDebug>
#include <QPointF>
#include <QRectF>
#include <QSettings>
#include <QString>

#include "scopecurvedata.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/data/analogscopesignal.hpp"
#include "src/data/analogsegment.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::dynamic_pointer_cast;
using std::set;
using std::shared_ptr;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

ScopeCurveData::ScopeCurveData(shared_ptr<sv::data::AnalogScopeSignal> signal) :
	BaseCurveData(CurveType::TimeCurve),
	signal_(signal),
	actual_segment_id_(0)
{
	connect(signal.get(), &sv::data::AnalogScopeSignal::segment_added,
		this, &ScopeCurveData::on_segment_added);
	connect(signal.get(), &sv::data::AnalogScopeSignal::samples_added,
		this, &ScopeCurveData::on_samples_added);
}

bool ScopeCurveData::is_equal(const BaseCurveData *other) const
{
	const ScopeCurveData *scd = dynamic_cast<const ScopeCurveData *>(other);
	if (scd == nullptr)
		return false;

	return signal_ == scd->signal();
}

QPointF ScopeCurveData::sample(size_t i) const
{
	//signal_data_->lock();

	auto segment = signal_->get_segment(actual_segment_id_);
	QPointF sample_point(segment->time_stride() * i, segment->get_sample(i));

	//qWarning() << "ScopeCurveData::sample(): " << i << " = " << sample_point
	//	<< " (time_stride = " << QString("%1").arg(segment->time_stride(), 10, 'f', -1, '0') << ")";

	//signal_data_->.unlock();

	return sample_point;
}

size_t ScopeCurveData::size() const
{
	// TODO: Synchronize x/y sample data
	return signal_->get_segment(actual_segment_id_)->sample_count();
}

QRectF ScopeCurveData::boundingRect() const
{
	/*
	qWarning() << "ScopeCurveData::boundingRect(): min time = "
		<< signal_->first_timestamp(relative_time_);
	qWarning() << "ScopeCurveData::boundingRect(): max time = "
		<< signal_->last_timestamp(relative_time_);
	qWarning() << "ScopeCurveData::boundingRect(): min value = "
		<< signal_->min_value();
	qWarning() << "ScopeCurveData::boundingRect(): max value = "
		<< signal_->max_value();
	*/

	auto segment = signal_->get_segment(actual_segment_id_);
	if (segment == nullptr) {
		qWarning() << "ScopeCurveData::boundingRect(): No last_segment found!";
		return QRectF(0, 0, 0, 0);
	}

	// top left, bottom right
	return QRectF(
		QPointF(.0, segment->get_min_max().second),
		QPointF(segment->time_stride() * segment->sample_count(),
				segment->get_min_max().first));
}

QPointF ScopeCurveData::closest_point(const QPointF &pos, double *dist) const
{
	(void)dist;
	const double x_value = pos.x();
	const int index_max = (int)size() - 1;

	// Corner cases
	if (index_max < 0)
		return QPointF(0, 0);
	if (x_value <= sample(0).x())
		return sample(0);
	if (x_value >= sample(index_max).x())
		return sample(index_max);

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

QString ScopeCurveData::name() const
{
	return signal_->display_name();
}

string ScopeCurveData::id_prefix() const
{
	return "scopecurve";
}

sv::data::Quantity ScopeCurveData::x_quantity() const
{
	return sv::data::Quantity::Time;
}

set<sv::data::QuantityFlag> ScopeCurveData::x_quantity_flags() const
{
	return set<data::QuantityFlag>();
}

sv::data::Unit ScopeCurveData::x_unit() const
{
	return sv::data::Unit::Second;
}

QString ScopeCurveData::x_unit_str() const
{
	return data::datautil::format_unit(x_unit());
}

QString ScopeCurveData::x_title() const
{
	return QString("%1 [%2]").
		arg(data::datautil::format_quantity(x_quantity()), x_unit_str());
}

sv::data::Quantity ScopeCurveData::y_quantity() const
{
	return signal_->quantity();
}

set<sv::data::QuantityFlag> ScopeCurveData::y_quantity_flags() const
{
	return signal_->quantity_flags();
}

sv::data::Unit ScopeCurveData::y_unit() const
{
	return signal_->unit();
}

QString ScopeCurveData::y_unit_str() const
{
	return data::datautil::format_unit(y_unit(), y_quantity_flags());
}

QString ScopeCurveData::y_title() const
{
	// Don't use only the unit, so we can add AC/DC to axis label.
	return QString("%1 [%2]").
		arg(data::datautil::format_quantity(y_quantity()), y_unit_str());
}

shared_ptr<sv::data::AnalogScopeSignal> ScopeCurveData::signal() const
{
	return signal_;
}

void ScopeCurveData::on_samples_added(uint32_t segment_id)
{
	(void)segment_id;
	Q_EMIT update_curve();
}

void ScopeCurveData::on_segment_added(uint32_t segment_id)
{
	actual_segment_id_ = segment_id;
	Q_EMIT reset_curve();
}

void ScopeCurveData::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	(void)settings;
	(void)origin_device;
	/*
	SettingsManager::save_signal(signal_, settings, origin_device);
	*/
}

ScopeCurveData *ScopeCurveData::init_from_settings(
	Session &session, QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	(void)session;
	(void)settings;
	(void)origin_device;
	return nullptr;
	/*
	auto signal = SettingsManager::restore_signal(
		session, settings, origin_device);
	if (!signal)
		return nullptr;

	return new ScopeCurveData(
		dynamic_pointer_cast<sv::data::AnalogScopeSignal>(signal));
	*/
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
