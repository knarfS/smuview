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

#ifndef WIDGETS_PLOT_HPP
#define WIDGETS_PLOT_HPP

#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot.h>
#include <qwt_interval.h>
#include <qwt_system_clock.h>

namespace sv {

namespace data {
class CurveData;
}

namespace widgets {

class Plot : public QwtPlot
{
	Q_OBJECT

public:
	Plot(data::CurveData *curve_data, QWidget * = NULL);
	virtual ~Plot();

	enum PlotModes
	{
	Additive = 0,
	Oscilloscope,
	Rolling,
	NumItems
	};

	virtual void replot();
	virtual bool eventFilter( QObject *, QEvent * );
	void setPlotInterval( int interval ) { m_plotInterval = interval; }
	void setPlotMode( Plot::PlotModes mode ) { m_plotMode = mode; }

public Q_SLOTS:
	void start();
	void stop();
	void setIntervalLength( double );

protected:
	virtual void showEvent( QShowEvent * );
	virtual void resizeEvent( QResizeEvent * );
	virtual void timerEvent( QTimerEvent * );

private:
	void updateCurve();
	void incrementInterval();

	data::CurveData *curve_data_;
	QwtPlotDirectPainter *m_valueDirectPainter;
	QwtPlotCurve *m_valueCurve;
	int m_paintedPoints;

	QwtInterval m_interval;
	double m_intervalLength;
	int m_timerId;
	int m_plotInterval;
	PlotModes m_plotMode;

	QwtSystemClock m_clock;
};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_PLOT_HPP
