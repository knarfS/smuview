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

#include <QDebug>
#include <QEvent>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_curve_fitter.h>
#include <qwt_painter.h>
#include <qwt_legend.h>

#include "src/data/signaldata.hpp"
#include "src/data/curvedata.hpp"
#include "plot.hpp"

namespace sv {
namespace widgets {

class Canvas: public QwtPlotCanvas
{
public:
	Canvas( QwtPlot *plot = NULL ):
		QwtPlotCanvas( plot )
	{
		// The backing store is important, when working with widget
		// overlays ( f.e rubberbands for zooming ).
		// Here we don't have them and the internal
		// backing store of QWidget is good enough.

		setPaintAttribute( QwtPlotCanvas::BackingStore, false );
		setBorderRadius( 10 );

		if ( QwtPainter::isX11GraphicsSystem() )
		{
#if QT_VERSION < 0x050000
			// Even if not liked by the Qt development, Qt::WA_PaintOutsidePaintEvent
			// works on X11. This has a nice effect on the performance.

			setAttribute( Qt::WA_PaintOutsidePaintEvent, true );
#endif

			// Disabling the backing store of Qt improves the performance
			// for the direct painter even more, but the canvas becomes
			// a native window of the window system, receiving paint events
			// for resize and expose operations. Those might be expensive
			// when there are many points and the backing store of
			// the canvas is disabled. So in this application
			// we better don't disable both backing stores.

			if ( testPaintAttribute( QwtPlotCanvas::BackingStore ) )
			{
				setAttribute( Qt::WA_PaintOnScreen, true );
				setAttribute( Qt::WA_NoSystemBackground, true );
			}
		}

		setupPalette();
	}

private:
	void setupPalette()
	{
		QPalette pal = palette();

#if QT_VERSION >= 0x040400
		QLinearGradient gradient;
		gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
		gradient.setColorAt( 0.0, QColor( 0, 49, 110 ) );
		gradient.setColorAt( 1.0, QColor( 0, 87, 174 ) );

		pal.setBrush( QPalette::Window, QBrush( gradient ) );
#else
		pal.setBrush( QPalette::Window, QBrush( color ) );
#endif

		// QPalette::WindowText is used for the curve color
		pal.setColor( QPalette::WindowText, Qt::green );

		setPalette( pal );
	}
};

Plot::Plot(data::CurveData *curve_data, QWidget *parent ):
	QwtPlot( parent ),
	curve_data_(curve_data),
	m_paintedPoints( 0 ),
	m_interval( 0.0, 30.0 ),
	m_timerId( -1 )
{
	m_valueDirectPainter = new QwtPlotDirectPainter();

	setAutoReplot( false );
	setCanvas( new Canvas() );

	plotLayout()->setAlignCanvasToScales( true );

	QwtLegend *legend = new QwtLegend;
	legend->setDefaultItemMode( QwtLegendData::Checkable );
	insertLegend( legend, QwtPlot::BottomLegend );

	// Time axis
	setAxisTitle( QwtPlot::xBottom, "Time [s]" );
	//setAxisScale( QwtAxisId( QwtAxis::xBottom, 0 ), m_interval.minValue(), m_interval.maxValue() ); // TODO: Multiaxis
	setAxisScale(0, m_interval.minValue(), m_interval.maxValue() );

	//setAxesCount( QwtPlot::yLeft, 2 ); // TODO: Multiaxis
	// Current axis
	//QwtAxisId currentAxisId = QwtAxisId( QwtAxis::yLeft, 0 ); // TODO: Multiaxis
	int currentAxisId = 0;
	//setAxisVisible( currentAxisId, true ); // TODO: Multiaxis
	setAxisTitle( currentAxisId, "Current [A]" );
	setAxisScale( currentAxisId, 0.0, 0.5 );

	// Voltage axis
	/*
	QwtAxisId voltageAxisId = QwtAxisId( QwtAxis::yLeft, 1 ); // TODO: Multiaxis
	setAxisVisible( voltageAxisId, true); // TODO: Multiaxis
	setAxisTitle( voltageAxisId, "Voltage [V]" );
	setAxisScale( voltageAxisId, 0.0, 5.0 );
	*/

	QwtPlotGrid *grid = new QwtPlotGrid();
	grid->setPen( Qt::gray, 0.0, Qt::DotLine );
	grid->enableX( true );
	grid->enableXMin( true );
	grid->enableY( true );
	grid->enableYMin( false );
	grid->attach( this );

	/*
	// SetCurrent
	m_setValueCurve = new QwtPlotCurve( "Set" );
	m_setValueCurve->setYAxis( currentAxisId );
	m_setValueCurve->setXAxis( QwtAxisId( QwtAxis::xBottom, 0 ) );
	m_setValueCurve->setStyle( QwtPlotCurve::Lines );
	m_setValueCurve->setPen( Qt::green, 2.0, Qt::SolidLine );
	m_setValueCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
	m_setValueCurve->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );
	//d_setCurrentCurve->setPaintAttribute( QwtPlotCurve::FilterPoints, true );
	m_setValueCurve->setData( new SetValueCurveData() );
	m_setValueCurve->attach( this );
	*/

	// Voltage
	/*
	m_currentCurve = new QwtPlotCurve( "Voltage" );
	m_currentCurve->setYAxis( voltageAxisId );
	//m_currentCurve->setXAxis( QwtAxisId( QwtAxis::xBottom, 0 ) ); // TODO: Multiaxis
	m_currentCurve->setXAxis( 0 );
	m_currentCurve->setStyle( QwtPlotCurve::Lines );
	m_currentCurve->setPen( Qt::green, 2.0, Qt::SolidLine );
	m_currentCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
	m_currentCurve->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );
	m_currentCurve->setData( new VoltageCurveData() );
	m_currentCurve->attach( this );
	*/

	// Value curve
	m_valueCurve = new QwtPlotCurve( "Current" );
	m_valueCurve->setYAxis( currentAxisId );
	//m_valueCurve->setXAxis( QwtAxisId( QwtAxis::xBottom, 0 ) ); // TODO: Multiaxis
	m_valueCurve->setXAxis( 0 );
	m_valueCurve->setStyle( QwtPlotCurve::Lines );
	m_valueCurve->setPen( Qt::red, 2.0, Qt::SolidLine );
	m_valueCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
	m_valueCurve->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );
	m_valueCurve->setData(curve_data_);
	//m_valueCurve->setRawSamples();
	m_valueCurve->attach( this );
}

Plot::~Plot()
{
	delete m_valueDirectPainter;
}

void Plot::start()
{
	m_clock.start();
	m_timerId = startTimer( m_plotInterval );
}

void Plot::stop()
{
	//d_clock.stop();
	killTimer( m_timerId );
}

void Plot::replot()
{
	//ReLoadProData::instance().lock();

	QwtPlot::replot();
	m_paintedPoints = curve_data_->size();

	//ReLoadProData::instance().unlock();
}

void Plot::setIntervalLength( double interval )
{
	m_intervalLength = interval;

	if ( interval > 0.0 && interval != m_interval.width() )
	{
		m_interval.setMaxValue( m_interval.minValue() + interval );
		setAxisScale( QwtPlot::xBottom,
			m_interval.minValue(), m_interval.maxValue() );

		replot();
	}
}

void Plot::updateCurve()
{
	//ReLoadProData::instance().lock(); // TODO
	//qDebug() << QString( "Plot::updateCurve(): ReLoadProData::instance().lock()" );

	const int numPoints = curve_data_->size();
	if ( numPoints > m_paintedPoints )
	{
		qWarning() << QString("Plot::updateCurve(): numPoints = %1, m_paintedPoints = %2").arg(numPoints).arg(m_paintedPoints);
		const bool doClip = !canvas()->testAttribute( Qt::WA_PaintOnScreen );
		if ( doClip )
		{
			qWarning() << QString("Plot::updateCurve(): doClip = %1").arg(doClip);
			/*
				Depending on the platform setting a clip might be an important
				performance issue. F.e. for Qt Embedded this reduces the
				part of the backing store that has to be copied out - maybe
				to an unaccelerated frame buffer device.
			*/

			//const QwtScaleMap xMap = canvasMap( m_setValueCurve->xAxis() );
			//const QwtScaleMap yMap = canvasMap( m_setValueCurve->yAxis() );
			const QwtScaleMap xMap = canvasMap( m_valueCurve->xAxis() );
			const QwtScaleMap yMap = canvasMap( m_valueCurve->yAxis() );

			QRectF br = qwtBoundingRect( *m_valueCurve->data(),
				m_paintedPoints - 1, numPoints - 1 );

			const QRect clipRect = QwtScaleMap::transform( xMap, yMap, br ).toRect();
			//m_setValueDirectPainter->setClipRegion( clipRect );
			//m_currentDirectPainter->setClipRegion( clipRect );
			m_valueDirectPainter->setClipRegion( clipRect );
		}

		/*
		m_setValueDirectPainter->drawSeries( m_setValueCurve,
			m_paintedPoints - 1, numPoints - 1 );
		*/

		/*
		m_currentDirectPainter->drawSeries( m_currentCurve,
			m_paintedPoints - 1, numPoints - 1 );
		*/

		m_valueDirectPainter->drawSeries( m_valueCurve,
			m_paintedPoints - 1, numPoints - 1 );

		m_paintedPoints = numPoints;
	}

	//qDebug() << QString( "Plot::updateCurve(): ReLoadProData::instance().unlock()" );
	//ReLoadProData::instance().unlock(); // TODO

	replot();
}

void Plot::incrementInterval()
{
	if ( m_plotMode == Plot::Additive )
	{
		m_interval = QwtInterval( m_interval.minValue(),
			m_interval.maxValue() + m_intervalLength );
	}
	else if ( m_plotMode == Plot::Oscilloscope )
	{
		m_interval = QwtInterval( m_interval.maxValue(),
			m_interval.maxValue() + m_interval.width() );
	}

	qWarning() << QString("Plot::incrementInterval(): -1-");

	//curve_data_->clearStaleValues( m_interval.minValue() ); // TODO

	if ( m_plotMode == Plot::Additive )
	{
		setAxisScale( QwtPlot::xBottom, m_interval.minValue(), m_interval.maxValue() );
	}
	else if ( m_plotMode == Plot::Oscilloscope )
	{
		// To avoid, that the grid is jumping, we disable
		// the autocalculation of the ticks and shift them
		// manually instead.

		QwtScaleDiv scaleDiv = axisScaleDiv( QwtPlot::xBottom );
		scaleDiv.setInterval( m_interval );

		for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
		{
			QList<double> ticks = scaleDiv.ticks( i );
			for ( int j = 0; j < ticks.size(); j++ )
			{
				ticks[j] += m_interval.width();
			}
			scaleDiv.setTicks( i, ticks );
		}
		setAxisScaleDiv( QwtPlot::xBottom, scaleDiv );

		m_paintedPoints = 0;
	}
	else if ( m_plotMode == Plot::Rolling )
	{

	}

	replot();
}

void Plot::timerEvent( QTimerEvent *event )
{
	if ( event->timerId() == m_timerId )
	{
		updateCurve();

		const double elapsed = m_clock.elapsed() / 1000.0;
		if ( elapsed > m_interval.maxValue() )
			incrementInterval();

		return;
	}

	QwtPlot::timerEvent( event );
}

void Plot::resizeEvent( QResizeEvent *event )
{
	m_valueDirectPainter->reset();
	QwtPlot::resizeEvent( event );
}

void Plot::showEvent( QShowEvent * )
{
	replot();
}

bool Plot::eventFilter( QObject *object, QEvent *event )
{
	if ( object == canvas() &&
		event->type() == QEvent::PaletteChange )
	{
		//m_setValueCurve->setPen( Qt::green, 2.0, Qt::SolidLine );
		//m_currentCurve->setPen( Qt::red, 2.0, Qt::SolidLine );
		m_valueCurve->setPen( Qt::green, 2.0, Qt::SolidLine );
	}

	return QwtPlot::eventFilter( object, event );
}

} // namespace widgets
} // namespace sv
