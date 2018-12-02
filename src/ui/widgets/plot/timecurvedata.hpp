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

#ifndef UI_WIDGETS_PLOT_TIMECURVEDATA_HPP
#define UI_WIDGETS_PLOT_TIMECURVEDATA_HPP

#include <memory>

#include <QPointF>
#include <QRectF>
#include <QString>

#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::shared_ptr;

namespace sv {

namespace data {
class AnalogSignal;
}

namespace ui {
namespace widgets {
namespace plot {

class TimeCurveData : public BaseCurveData
{

public:
	TimeCurveData(shared_ptr<sv::data::AnalogSignal> signal);

	QPointF sample(size_t i) const override;
	size_t size() const override;
	QRectF boundingRect() const override;

	QPointF closest_point(const QPointF &pos, double *dist) const override;
	QString name() const override;
	QString x_data_quantity() const override;
	QString x_data_unit() const override;
	QString x_data_title() const override;
	QString y_data_quantity() const override;
	QString y_data_unit() const override;
	QString y_data_title() const override;

private:
	shared_ptr<sv::data::AnalogSignal> signal_;

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_TIMECURVEDATA_HPP
