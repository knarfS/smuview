/*
 * This file is part of the SmuView project.
 * This file is based on the QWT EventFilter Example.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <math.h>

#include <QEvent>
#include <QDebug>
#include <QMouseEvent>
#include <QObject>
#include <QPoint>
#include <QWheelEvent>

#include <qwt_scale_draw.h>
#include <qwt_scale_map.h>
#include <qwt_scale_widget.h>

#include "plotscalepicker.hpp"
#include "src/widgets/plot.hpp"

namespace sv {
namespace widgets {

PlotScalePicker::PlotScalePicker(Plot *plot) :
	QObject(plot),
	plot_(plot),
	wheel_factor_(0.9)
{
	for (uint i = 0; i < QwtPlot::axisCnt; i++) {
		QwtScaleWidget *scale_widget = plot->axisWidget(i);
		if (scale_widget)
			scale_widget->installEventFilter(this);
	}
}

bool PlotScalePicker::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress) {
		QwtScaleWidget *scale_widget = qobject_cast<QwtScaleWidget *>(object);
		if (scale_widget) {
			QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
			if (mouse_event->buttons() & Qt::LeftButton) {
				QPoint pos = mouse_event->pos();
				switch (scale_widget->alignment()) {
				case QwtScaleDraw::LeftScale:
				case QwtScaleDraw::RightScale:
					last_pan_p_value_ = pos.y();
					break;
				case QwtScaleDraw::BottomScale:
				case QwtScaleDraw::TopScale:
					last_pan_p_value_ = pos.x();
					break;
				}

				return true;
			}
		}
	}
	else if (event->type() == QEvent::MouseMove) {
		QwtScaleWidget *scale_widget = qobject_cast<QwtScaleWidget *>(object);
		if (scale_widget) {
			QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
			if (mouse_event->buttons() & Qt::LeftButton) {
				QPoint pos = mouse_event->pos();
				int axis_id = -1;
				int p_value = 0;
				switch (scale_widget->alignment()) {
				case QwtScaleDraw::LeftScale:
					axis_id = QwtPlot::yLeft;
					p_value = pos.y();
					plot_->set_y_axis_fixed(true);
					break;
				case QwtScaleDraw::RightScale:
					axis_id = QwtPlot::yRight;
					p_value = pos.y();
					plot_->set_y_axis_fixed(true);
					break;
				case QwtScaleDraw::BottomScale:
					axis_id = QwtPlot::xBottom;
					p_value = pos.x();
					plot_->set_x_axis_fixed(true);
					break;
				case QwtScaleDraw::TopScale:
					axis_id = QwtPlot::xTop;
					p_value = pos.x();
					plot_->set_x_axis_fixed(true);
					break;
				}

				const bool auto_replot = plot_->autoReplot();
				plot_->setAutoReplot(false);

				const QwtScaleMap scale_map =
					scale_widget->scaleDraw()->scaleMap();
				const double p1 = scale_map.transform(
					plot_->axisScaleDiv(axis_id).lowerBound());
				const double p2 = scale_map.transform(
					plot_->axisScaleDiv(axis_id).upperBound());

				int p_diff = p_value - last_pan_p_value_;
				double s1 = scale_map.invTransform(p1 - p_diff);
				double s2 = scale_map.invTransform(p2 - p_diff);
				last_pan_p_value_ = p_value;

				plot_->setAxisScale(axis_id, s1, s2);
				plot_->setAutoReplot(auto_replot);
				plot_->replot();

				return true;
			}
		}
	}
	else if (event->type() == QEvent::Wheel) {
		QwtScaleWidget *scale_widget = qobject_cast<QwtScaleWidget *>(object);
		if (scale_widget) {
			QWheelEvent *wheel_event = static_cast<QWheelEvent *>(event);
			if (wheel_event) {
				double factor = pow(
					wheel_factor_, abs(wheel_event->delta() / 120.0));
				if (wheel_event->delta() > 0)
					factor = 1 / factor;
				factor = abs(factor);
				if (factor == 1.0 || factor == 0.0)
					return true;

				const bool auto_replot = plot_->autoReplot();
				plot_->setAutoReplot(false);

				int axis_id = -1;
				switch (scale_widget->alignment()) {
				case QwtScaleDraw::LeftScale:
					axis_id = QwtPlot::yLeft;
					break;
				case QwtScaleDraw::RightScale:
					axis_id = QwtPlot::yRight;
					break;
				case QwtScaleDraw::BottomScale:
					axis_id = QwtPlot::xBottom;
					break;
				case QwtScaleDraw::TopScale:
					axis_id = QwtPlot::xTop;
					break;
				}

				const QwtScaleMap scale_map = plot_->canvasMap(axis_id);
				double v1 = scale_map.s1();
				double v2 = scale_map.s2();
				if (scale_map.transformation()) {
					// the coordinate system of the paint device is always linear
					v1 = scale_map.transform(v1); // scale_map.p1()
					v2 = scale_map.transform(v2); // scale_map.p2()
				}
				const double center = 0.5 * (v1 + v2);
				const double width_2 = 0.5 * (v2 - v1) * factor;
				v1 = center - width_2;
				v2 = center + width_2;
				if (scale_map.transformation()) {
					v1 = scale_map.invTransform(v1);
					v2 = scale_map.invTransform(v2);
				}

				plot_->setAxisScale(axis_id, v1, v2);
				plot_->replot();
				plot_->setAutoReplot(auto_replot);

				return true;
			}
		}
	}

	return QObject::eventFilter(object, event);
}

} // namespace widgets
} // namespace sv

