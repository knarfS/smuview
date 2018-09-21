/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
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
#include <QShowEvent>
#include <QWidget>

#include "popup.hpp"

using std::max;
using std::min;

namespace sv {
namespace ui {
namespace widgets {

const unsigned int Popup::ArrowLength = 10;
const unsigned int Popup::ArrowOverlap = 3;
const unsigned int Popup::MarginWidth = 6;

Popup::Popup(QWidget *parent) :
	QWidget(parent, Qt::Popup | Qt::FramelessWindowHint),
	point_(),
	pos_(PopupPosition::Left)
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
	point_ = point, pos_ = pos;

	setContentsMargins(
		MarginWidth + ((pos == PopupPosition::Right) ? ArrowLength : 0),
		MarginWidth + ((pos == PopupPosition::Bottom) ? ArrowLength : 0),
		MarginWidth + ((pos == PopupPosition::Left) ? ArrowLength : 0),
		MarginWidth + ((pos == PopupPosition::Top) ? ArrowLength : 0));
}

bool Popup::eventFilter(QObject *obj, QEvent *event)
{
	QKeyEvent *keyEvent;

	(void)obj;

	if (event->type() == QEvent::KeyPress) {
		keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Enter ||
				keyEvent->key() == Qt::Key_Return) {
			close();
			return true;
		}
	}

	return false;
}

void Popup::show()
{
	QLineEdit* le;

	QWidget::show();

	// We want to close the popup when the Enter key was
	// pressed and the first editable widget had focus.
	if ((le = this->findChild<QLineEdit*>())) {

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
		if (point_.x() > x())
			return false;
		return true;

	case PopupPosition::Bottom:
		if (point_.y() > y())
			return false;
		return true;

	case PopupPosition::Left:
		if (point_.x() < (x() + width()))
			return false;
		return true;

	case PopupPosition::Top:
		if (point_.y() < (y() + height()))
			return false;
		return true;
	}

	return true;
}

QPolygon Popup::arrow_polygon() const
{
	QPolygon poly;

	const QPoint p = mapFromGlobal(point_);
	const int l = ArrowLength + ArrowOverlap;

	switch (pos_) {
	case PopupPosition::Right:
		poly << QPoint(p.x() + l, p.y() - l);
		break;

	case PopupPosition::Bottom:
		poly << QPoint(p.x() - l, p.y() + l);
		break;

	case PopupPosition::Left:
	case PopupPosition::Top:
		poly << QPoint(p.x() - l, p.y() - l);
		break;
	}

	poly << p;

	switch (pos_) {
	case PopupPosition::Right:
	case PopupPosition::Bottom:
		poly << QPoint(p.x() + l, p.y() + l);
		break;

	case PopupPosition::Left:
		poly << QPoint(p.x() - l, p.y() + l);
		break;

	case PopupPosition::Top:
		poly << QPoint(p.x() + l, p.y() - l);
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

	const unsigned int r = MarginWidth;
	const unsigned int d = 2 * r;
	return QRegion(rect.adjusted(r, 0, -r, 0)).united(
		QRegion(rect.adjusted(0, r, 0, -r))).united(
		QRegion(rect.left(), rect.top(), d, d,
			QRegion::Ellipse)).united(
		QRegion(rect.right() - d, rect.top(), d, d,
			QRegion::Ellipse)).united(
		QRegion(rect.left(), rect.bottom() - d, d, d,
			QRegion::Ellipse)).united(
		QRegion(rect.right() - d, rect.bottom() - d, d, d,
			QRegion::Ellipse));
}

QRegion Popup::popup_region() const
{
	if (space_for_arrow())
		return arrow_region().united(bubble_region());
	else
		return bubble_region();
}

void Popup::reposition_widget()
{
	QPoint o;

	const QRect screen_rect = QApplication::desktop()->availableGeometry(
		QApplication::desktop()->screenNumber(point_));

	if (pos_ == PopupPosition::Right || pos_ == PopupPosition::Left)
		o.ry() = -height() / 2;
	else
		o.rx() = -width() / 2;

	if (pos_ == PopupPosition::Left)
		o.rx() = -width();
	else if (pos_ == PopupPosition::Top)
		o.ry() = -height();

	o += point_;
	move(max(min(o.x(), screen_rect.right() - width()), screen_rect.left()),
		max(min(o.y(), screen_rect.bottom() - height()), screen_rect.top()));
}

void Popup::closeEvent(QCloseEvent *)
{
	closed();
}

void Popup::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	const QColor outline_color(QApplication::palette().color(
		QPalette::Dark));

	// Draw the bubble
	const QRegion b = bubble_region();
	const QRegion bubble_outline = QRegion(rect()).subtracted(
		b.translated(1, 0).intersected(b.translated(0, 1).intersected(
		b.translated(-1, 0).intersected(b.translated(0, -1)))));

	painter.setPen(Qt::NoPen);
	painter.setBrush(QApplication::palette().brush(QPalette::Window));
	painter.drawRect(rect());

	// Draw the arrow
	if (!space_for_arrow())
		return;

	const QPoint ArrowOffsets[] = {
		QPoint(1, 0), QPoint(0, -1), QPoint(-1, 0), QPoint(0, 1)};

	const QRegion a(arrow_region());
	const QRegion arrow_outline = a.subtracted(
		a.translated(ArrowOffsets[static_cast<int>(pos_)]));

	painter.setClipRegion(bubble_outline.subtracted(a).united(arrow_outline));
	painter.setBrush(outline_color);
	painter.drawRect(rect());
}

void Popup::resizeEvent(QResizeEvent *)
{
	reposition_widget();
	setMask(popup_region());
}

void Popup::mouseReleaseEvent(QMouseEvent *event)
{
	assert(event);

	// We need our own out-of-bounds click handler because QWidget counts
	// the drop-shadow region as inside the widget
	if (!bubble_rect().contains(event->pos()))
		close();
}

void Popup::showEvent(QShowEvent *)
{
	reposition_widget();
}

} // namespace widgets
} // namespace ui
} // namespace sv
