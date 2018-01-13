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

#ifndef DATA_BASECURVE_HPP
#define DATA_BASECURVE_HPP

#include <memory>
#include <vector>

#include <QPointer>
#include <qwt_series_data.h>

using std::shared_ptr;
using std::vector;

namespace sv {
namespace data {

class BaseCurve : public QwtSeriesData<QPointF>
{

public:
	BaseCurve();

	virtual QPointF sample( size_t i ) const = 0;
	virtual size_t size() const = 0;
	virtual QRectF boundingRect() const = 0;

	virtual QString x_data_quantity() const = 0;
	virtual QString x_data_unit() const = 0;
	virtual QString x_data_title() const = 0;
	virtual QString y_data_quantity() const = 0;
	virtual QString y_data_unit() const = 0;
	virtual QString y_data_title() const = 0;

};

} // namespace data
} // namespace sv

#endif // DATA_BASECURVE_HPP
