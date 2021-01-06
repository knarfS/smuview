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

#include <set>

#include <QSettings>
#include <QString>
#include <QtGlobal>
#include <QVariant>

#include <qwt_series_data.h>

#include "basecurvedata.hpp"
#include "src/data/datautil.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

BaseCurveData::BaseCurveData(CurveType curve_type) :
	QwtSeriesData<QPointF>(),
	type_(curve_type),
	relative_time_(true)
{
}

CurveType BaseCurveData::type() const
{
	return type_;
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
