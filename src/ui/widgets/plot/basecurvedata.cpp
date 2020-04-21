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

#include <QColor>
#include <QtGlobal>
#if QT_VERSION >= 0x050A00
	#include <QRandomGenerator>
#endif

#include <qwt_series_data.h>

#include "basecurvedata.hpp"
#include "src/data/datautil.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

BaseCurveData::BaseCurveData(CurveType curve_type) :
	QwtSeriesData<QPointF>(),
	curve_type_(curve_type),
	relative_time_(true)
{
}

CurveType BaseCurveData::curve_type() const
{
	return curve_type_;
}

QColor BaseCurveData::color() const
{
	// TODO: Implement a better way for color configuration of curves!

	if (y_quantity() == sv::data::Quantity::Voltage &&
			y_quantity_flags().count(sv::data::QuantityFlag::DC) > 0)
		return Qt::red;
	if (y_quantity() == sv::data::Quantity::Voltage &&
			y_quantity_flags().count(sv::data::QuantityFlag::AC) > 0)
		return Qt::darkRed;
	if (y_quantity() == sv::data::Quantity::Voltage)
		// Fallback for Voltage without quantity flag
		return Qt::red;
	if (y_quantity() == sv::data::Quantity::Current &&
			y_quantity_flags().count(sv::data::QuantityFlag::DC) > 0)
		return Qt::green;
	if (y_quantity() == sv::data::Quantity::Current &&
			y_quantity_flags().count(sv::data::QuantityFlag::AC) > 0)
		return Qt::darkGreen;
	if (y_quantity() == sv::data::Quantity::Current)
		// Fallback for current without quantity flag
		return Qt::green;
	if (y_quantity() == sv::data::Quantity::Resistance)
		return Qt::cyan;
	if (y_quantity() == sv::data::Quantity::Power)
		return Qt::yellow;
	if (y_quantity() == sv::data::Quantity::Work)
		return Qt::darkYellow;
	if (y_quantity() == sv::data::Quantity::Temperature)
		return Qt::darkCyan;
	if (y_quantity() == sv::data::Quantity::Capacitance)
		return Qt::gray;
	if (y_quantity() == sv::data::Quantity::Frequency)
		return Qt::magenta;
	if (y_quantity() == sv::data::Quantity::Time)
		return Qt::darkMagenta;
	if (y_quantity() == sv::data::Quantity::PowerFactor)
		return Qt::lightGray;

#if QT_VERSION >= 0x050A00
	return QColor::fromRgb(QRandomGenerator::global()->generate());
#else
	return QColor::fromRgb(qrand());
#endif

	/*
	 * Unused colors:
		Qt::blue
		Qt::darkBlue
		Qt::darkGray
		Qt::white
		Qt::black
	*/
}

void BaseCurveData::set_relative_time(bool is_relative_time)
{
	relative_time_ = is_relative_time;
}

bool BaseCurveData::is_relative_time() const
{
	return relative_time_;
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
