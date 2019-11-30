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

#include <QColor>
#include <QRandomGenerator>
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
	if (y_quantity() == sv::data::Quantity::Voltage &&
			y_quantity_flags().count(sv::data::QuantityFlag::DC))
		return Qt::red;
	else if (y_quantity() == sv::data::Quantity::Voltage &&
			y_quantity_flags().count(sv::data::QuantityFlag::AC))
		return Qt::darkRed;
	else if (y_quantity() == sv::data::Quantity::Voltage)
		// Fallback for Voltage without quantity flag
		return Qt::red;
	else if (y_quantity() == sv::data::Quantity::Current &&
			y_quantity_flags().count(sv::data::QuantityFlag::DC))
		return Qt::green;
	else if (y_quantity() == sv::data::Quantity::Current &&
			y_quantity_flags().count(sv::data::QuantityFlag::AC))
		return Qt::darkGreen;
	else if (y_quantity() == sv::data::Quantity::Current)
		// Fallback for current without quantity flag
		return Qt::green;
	else if (y_quantity() == sv::data::Quantity::Resistance)
		return Qt::cyan;
	else if (y_quantity() == sv::data::Quantity::Power)
		return Qt::yellow;
	else if (y_quantity() == sv::data::Quantity::Work)
		return Qt::darkYellow;
	else if (y_quantity() == sv::data::Quantity::Temperature)
		return Qt::darkCyan;
	else if (y_quantity() == sv::data::Quantity::Capacitance)
		return Qt::gray;
	else if (y_quantity() == sv::data::Quantity::Frequency)
		return Qt::magenta;
	else if (y_quantity() == sv::data::Quantity::Time)
		return Qt::darkMagenta;
	else if (y_quantity() == sv::data::Quantity::PowerFactor)
		return Qt::lightGray;
	else
		return QColor::fromRgb(QRandomGenerator::global()->generate());

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
