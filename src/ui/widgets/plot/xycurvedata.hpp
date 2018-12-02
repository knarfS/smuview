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

#ifndef UI_WIDGETS_PLOT_XYCURVE_HPP
#define UI_WIDGETS_PLOT_XYCURVE_HPP

#include <memory>

#include <QPointF>
#include <QRectF>
#include <QString>

#include "src/ui/widgets/plot/basecurve.hpp"

using std::shared_ptr;

namespace sv {

namespace data {
class AnalogSignal;
}

namespace ui {
namespace widgets {
namespace plot {

class XYCurve : public BaseCurve
{

public:
	XYCurve(shared_ptr<sv::data::AnalogSignal> x_signal,
		shared_ptr<sv::data::AnalogSignal> y_signal);

	QPointF sample(size_t i) const;
	size_t size() const;
	QRectF boundingRect() const;

	QString name() const;
	QString x_data_quantity() const;
	QString x_data_unit() const;
	QString x_data_title() const;
	QString y_data_quantity() const;
	QString y_data_unit() const;
	QString y_data_title() const;

private:
	shared_ptr<sv::data::AnalogSignal> x_signal_;
	shared_ptr<sv::data::AnalogSignal> y_signal_;

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_XYCURVE_HPP
