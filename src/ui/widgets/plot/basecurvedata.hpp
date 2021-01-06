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

#ifndef UI_WIDGETS_PLOT_BASECURVEDATA_HPP
#define UI_WIDGETS_PLOT_BASECURVEDATA_HPP

#include <memory>
#include <set>
#include <string>

#include <QColor>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QSettings>
#include <QString>
#include <qwt_series_data.h>

#include "src/data/datautil.hpp"

using std::set;
using std::shared_ptr;
using std::string;

namespace sv {

namespace devices {
class BaseDevice;
}

namespace ui {
namespace widgets {
namespace plot {

enum class CurveType {
	TimeCurve,
	XYCurve
};

class BaseCurveData : public QObject, public QwtSeriesData<QPointF>
{
	Q_OBJECT

public:
	explicit BaseCurveData(CurveType curve_type);
	virtual ~BaseCurveData() = default;

	CurveType type() const;
	virtual QString name() const = 0;
	virtual string id_prefix() const = 0;
	void set_relative_time(bool is_relative_time);
	bool is_relative_time() const;

	virtual bool is_equal(const BaseCurveData *other) const = 0;

	virtual QPointF sample(size_t i) const = 0;
	virtual size_t size() const = 0;
	virtual QRectF boundingRect() const = 0;

	virtual QPointF closest_point(const QPointF &pos, double *dist) const = 0;
	virtual sv::data::Quantity x_quantity() const = 0;
	virtual set<sv::data::QuantityFlag> x_quantity_flags() const = 0;
	virtual sv::data::Unit x_unit() const = 0;
	virtual QString x_unit_str() const = 0;
	virtual QString x_title() const = 0;
	virtual sv::data::Quantity y_quantity() const = 0;
	virtual set<sv::data::QuantityFlag> y_quantity_flags() const = 0;
	virtual sv::data::Unit y_unit() const = 0;
	virtual QString y_unit_str() const = 0;
	virtual QString y_title() const = 0;

	virtual void save_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device) const = 0;

protected:
	const CurveType type_;
	bool relative_time_;

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_BASECURVEDATA_HPP
