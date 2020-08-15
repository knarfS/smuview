/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <QPointF>
#include <QRectF>
#include <QSettings>
#include <QString>

#include "timecurvedata.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::dynamic_pointer_cast;
using std::set;
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
	if (tcd == nullptr)
		return false;

	return signal_ == tcd->signal();
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
	return signal_->sample_count();
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

QString TimeCurveData::name() const
{
	return signal_->display_name();
}

string TimeCurveData::id_prefix() const
{
	return "timecurve";
}

sv::data::Quantity TimeCurveData::x_quantity() const
{
	return sv::data::Quantity::Time;
}

set<sv::data::QuantityFlag> TimeCurveData::x_quantity_flags() const
{
	return set<data::QuantityFlag>();
}

sv::data::Unit TimeCurveData::x_unit() const
{
	return sv::data::Unit::Second;
}

QString TimeCurveData::x_unit_str() const
{
	return data::datautil::format_unit(x_unit());
}

QString TimeCurveData::x_title() const
{
	return QString("%1 [%2]").
		arg(data::datautil::format_quantity(x_quantity())).
		arg(x_unit_str());
}

sv::data::Quantity TimeCurveData::y_quantity() const
{
	return signal_->quantity();
}

set<sv::data::QuantityFlag> TimeCurveData::y_quantity_flags() const
{
	return signal_->quantity_flags();
}

sv::data::Unit TimeCurveData::y_unit() const
{
	return signal_->unit();
}

QString TimeCurveData::y_unit_str() const
{
	return data::datautil::format_unit(y_unit(), y_quantity_flags());
}

QString TimeCurveData::y_title() const
{
	// Don't use only the unit, so we can add AC/DC to axis label.
	return QString("%1 [%2]").
		arg(data::datautil::format_quantity(y_quantity())).
		arg(y_unit_str());
}

shared_ptr<sv::data::AnalogTimeSignal> TimeCurveData::signal() const
{
	return signal_;
}

void TimeCurveData::save_settings(QSettings &settings) const
{
	SettingsManager::save_signal(signal_, settings);
}

TimeCurveData *TimeCurveData::init_from_settings(
	Session &session, QSettings &settings)
{
	auto signal = SettingsManager::restore_signal(session, settings);
	if (!signal)
		return nullptr;

	return new TimeCurveData(
		dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal));
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
