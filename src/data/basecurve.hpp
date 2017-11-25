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

#include <QPointer>
#include <qwt_series_data.h>

using std::shared_ptr;

namespace sv {
namespace data {

class AnalogData;

class BaseCurve : public QwtSeriesData<QPointF>
{

public:
	BaseCurve(shared_ptr<AnalogData> x_signal_data,
		shared_ptr<AnalogData> y_signal_data);

	virtual QPointF sample( size_t i ) const;
	virtual size_t size() const;
	virtual QRectF boundingRect() const;
	virtual QString x_signal_quantity() const;
	virtual QString x_signal_unit() const;
	virtual QString x_signal_title() const;
	virtual QString y_signal_quantity() const;
	virtual QString y_signal_unit() const;
	virtual QString y_signal_title() const;

private:
	shared_ptr<AnalogData> x_signal_data_;
	shared_ptr<AnalogData> y_signal_data_;

};

} // namespace data
} // namespace sv

#endif // DATA_BASECURVE_HPP
