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

#ifndef UI_WIDGETS_PLOT_BASECURVE_HPP
#define UI_WIDGETS_PLOT_BASECURVE_HPP

#include <QColor>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <qwt_series_data.h>

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

enum class CurveType {
	TimeCurve,
	XYCurve
};

class BaseCurve : public QwtSeriesData<QPointF>
{

public:
	BaseCurve(CurveType curve_type);
	virtual ~BaseCurve() = default;

	CurveType curve_type() const;
	QColor color() const;
	void set_relative_time(bool is_relative_time);
	bool is_relative_time() const;

	virtual QPointF sample(size_t i) const = 0;
	virtual size_t size() const = 0;
	virtual QRectF boundingRect() const = 0;

	virtual QString name() const = 0;
	virtual QString x_data_quantity() const = 0;
	virtual QString x_data_unit() const = 0;
	virtual QString x_data_title() const = 0;
	virtual QString y_data_quantity() const = 0;
	virtual QString y_data_unit() const = 0;
	virtual QString y_data_title() const = 0;

protected:
	const CurveType curve_type_;
	QColor color_;
	bool relative_time_;

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_BASECURVE_HPP
