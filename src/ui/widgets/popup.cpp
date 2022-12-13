/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <algorithm>
#include <cassert>

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QPaintEvent>
#include <QPoint>
#include <QPolygon>
#include <QRect>
#include <QRegion>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QWidget>

#include "popup.hpp"

using std::max;
using std::min;

namespace sv {
namespace ui {
namespace widgets {

const int Popup::ArrowLength = 10;
const int Popup::ArrowOverlap = 3;
const int Popup::MarginWidth = 6;

Popup::Popup(QWidget *parent) :
	QWidget(parent, Qt::Popup | Qt::FramelessWindowHint),
	pos_(PopupPosition::Left),
	mouse_pressed_(false)
{
}

const QPoint& Popup::point() const
{
	return point_;
}

PopupPosition Popup::position() const
{
	return pos_;
}

void Popup::set_position(const QPoint point, PopupPosition pos)
{
	point_ = point;
	pos_ = pos;

	setContentsMargins(
		MarginWidth + ((pos == PopupPosition::Right) ? ArrowLength : 0),
		MarginWidth + ((pos == PopupPosition::Bottom) ? ArrowLength : 0),
		MarginWidth + ((pos == PopupPosition::Left) ? ArrowLength : 0),
		MarginWidth + ((pos == PopupPosition::Top) ? ArrowLength : 0));
}

bool Popup::eventFilter(QObject *obj, QEvent *event)
{
	(void)obj;
	(void)event;

	return false;

	/*
	 * Deactivaded, because when catching key_Enter and _key_Return in here,
	 * the retunPressed() signal in AxisPopup isn't working correctly.

	QKeyEvent *key_event;

	(void)obj;

	if (event->type() == QEvent::KeyPress) {
		key_event = static_cast<QKeyEvent*>(event);
		if (key_event->key() == Qt::Key_Enter ||
				key_event->key() == Qt::Key_Return) {
			close();
			return true;
		}
	}

	return false;
	*/
}

void Popup::show()
{
	QWidget::show();

	// We want to close the popup when the Enter key was
	// pressed and the first editable widget had focus.
	QLineEdit *le = this->findChild<QLineEdit*>();
	if (le) {
		// For combo boxes we need to hook into the parent of
		// the line edit (i.e. the QComboBox). For edit boxes
		// we hook into the widget directly.
		if (le->parent()->metaObject()->className() ==
				this->metaObject()->className())
			le->installEventFilter(this);
		else
			le->parent()->installEventFilter(this);

		le->selectAll();
		le->setFocus();
	}
}

bool Popup::space_for_arrow() const
{
	// Check if there is room for the arrow
	switch (pos_) {
	case PopupPosition::Right:
		return point_.x() <= x();

	case PopupPosition::Bottom:
		return point_.y() <= y();

	case PopupPosition::Left:
		return point_.x() >= (x() + width());

	case PopupPosition::Top:
		return point_.y() >= (y() + height());
	}

	return true;
}

QPolygon Popup::arrow_polygon() const
{
	QPolygon poly;

	const QPoint widget_point = mapFromGlobal(point_);
	const int total_len = ArrowLength + ArrowOverlap;

	switch (pos_) {
	case PopupPosition::Right:
		poly << QPoint(widget_point.x() + total_len, widget_point.y() - total_len);
		break;

	case PopupPosition::Bottom:
		poly << QPoint(widget_point.x() - total_len, widget_point.y() + total_len);
		break;

	case PopupPosition::Left:
	case PopupPosition::Top:
		poly << QPoint(widget_point.x() - total_len, widget_point.y() - total_len);
		break;
	}

	poly << widget_point;

	switch (pos_) {
	case PopupPosition::Right:
	case PopupPosition::Bottom:
		poly << QPoint(widget_point.x() + total_len, widget_point.y() + total_len);
		break;

	case PopupPosition::Left:
		poly << QPoint(widget_point.x() - total_len, widget_point.y() + total_len);
		break;

	case PopupPosition::Top:
		poly << QPoint(widget_point.x() + total_len, widget_point.y() - total_len);
		break;
	}

	return poly;
}

QRegion Popup::arrow_region() const
{
	return QRegion(arrow_polygon());
}

QRect Popup::bubble_rect() const
{
	return QRect(
		QPoint((pos_ == PopupPosition::Right) ? ArrowLength : 0,
			(pos_ == PopupPosition::Bottom) ? ArrowLength : 0),
		QSize(
			width() -
				((pos_ == PopupPosition::Left || pos_ == PopupPosition::Right) ?
					ArrowLength : 0),
			height() -
				((pos_ == PopupPosition::Top || pos_ == PopupPosition::Bottom) ?
					ArrowLength : 0)));
}

QRegion Popup::bubble_region() const
{
	const QRect rect(bubble_rect());

	const int radius = MarginWidth;
	const int diameter = 2 * radius;
	return QRegion(rect.adjusted(radius, 0, -radius, 0))
		.united(QRegion(rect.adjusted(0, radius, 0, -radius)))
		.united(QRegion(rect.left(), rect.top(), diameter, diameter,
			QRegion::Ellipse))
		.united(QRegion(rect.right() - diameter, rect.top(), diameter, diameter,
			QRegion::Ellipse))
		.united(QRegion(rect.left(), rect.bottom() - diameter, diameter, diameter,
			QRegion::Ellipse))
		.united(QRegion(rect.right() - diameter, rect.bottom() - diameter,
			diameter, diameter, QRegion::Ellipse));
}

QRegion Popup::popup_region() const
{
	if (space_for_arrow())
		return arrow_region().united(bubble_region());
	else // NOLINT
		return bubble_region();
}

void Popup::reposition_widget()
{
	QPoint new_pos;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
	QScreen *screen = QGuiApplication::screenAt(point_);
	if(!screen)
		return;
	const QRect screen_rect = screen->availableGeometry();
#else
	const QRect screen_rect = QApplication::desktop()->availableGeometry(
		QApplication::desktop()->screenNumber(point_));
#endif

	if (pos_ == PopupPosition::Right || pos_ == PopupPosition::Left)
		new_pos.ry() = -height() / 2;
	else
		new_pos.rx() = -width() / 2;

	if (pos_ == PopupPosition::Left)
		new_pos.rx() = -width();
	else if (pos_ == PopupPosition::Top)
		new_pos.ry() = -height();

	new_pos += point_;
	move(max(min(new_pos.x(), screen_rect.right() - width()), screen_rect.left()),
		max(min(new_pos.y(), screen_rect.bottom() - height()), screen_rect.top()));
}

void Popup::closeEvent(QCloseEvent *event)
{
	(void)event;
	Q_EMIT closed();
}

void Popup::paintEvent(QPaintEvent *event)
{
	(void)event;

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	const QColor outline_color(QApplication::palette().color(
		QPalette::Dark));

	// Draw the bubble
	const QRegion bubble = bubble_region();
	const QRegion bubble_outline = QRegion(rect()).subtracted(
		bubble.translated(1, 0).intersected(bubble.translated(0, 1).intersected(
		bubble.translated(-1, 0).intersected(bubble.translated(0, -1)))));

	painter.setPen(Qt::NoPen);
	painter.setBrush(QApplication::palette().brush(QPalette::Window));
	painter.drawRect(rect());

	// Draw the arrow
	if (!space_for_arrow())
		return;

	const QPoint ArrowOffsets[] = {
		QPoint(1, 0), QPoint(0, -1), QPoint(-1, 0), QPoint(0, 1)};

	const QRegion arrow(arrow_region());
	const QRegion arrow_outline = arrow.subtracted(
		arrow.translated(ArrowOffsets[static_cast<int>(pos_)]));

	painter.setClipRegion(bubble_outline.subtracted(arrow).united(arrow_outline));
	painter.setBrush(outline_color);
	painter.drawRect(rect());
}

void Popup::resizeEvent(QResizeEvent *event)
{
	(void)event;
	reposition_widget();
	setMask(popup_region());
}


void Popup::mousePressEvent(QMouseEvent *event)
{
	(void)event;
	mouse_pressed_ = true;
}

void Popup::mouseReleaseEvent(QMouseEvent *event)
{
	assert(event);

	if (!mouse_pressed_)
		return;

	mouse_pressed_ = false;

	// We need our own out-of-bounds click handler because QWidget counts
	// the drop-shadow region as inside the widget
	if (!bubble_rect().contains(event->pos()))
		close();
}

void Popup::showEvent(QShowEvent *event)
{
	(void)event;
	reposition_widget();
}

} // namespace widgets
} // namespace ui
} // namespace sv
