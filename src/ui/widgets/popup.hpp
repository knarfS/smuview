/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_POPUP_HPP
#define UI_WIDGETS_POPUP_HPP

#include <QCloseEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QObject>
#include <QPaintEvent>
#include <QPoint>
#include <QPolygon>
#include <QRect>
#include <QRegion>
#include <QResizeEvent>
#include <QShowEvent>
#include <QWidget>

namespace sv {
namespace ui {
namespace widgets {

enum class PopupPosition : int
{
	Right,
	Top,
	Left,
	Bottom
};

class Popup : public QWidget
{
	Q_OBJECT

private:
	static const unsigned int ArrowLength;
	static const unsigned int ArrowOverlap;
	static const unsigned int MarginWidth;

public:
	Popup(QWidget *parent);

	const QPoint &point() const;
	PopupPosition position() const;
	void set_position(const QPoint point, PopupPosition pos);
	bool eventFilter(QObject *obj, QEvent *event) override;
	void show();

private:
	bool space_for_arrow() const;
	QPolygon arrow_polygon() const;
	QRegion arrow_region() const;
	QRect bubble_rect() const;
	QRegion bubble_region() const;
	QRegion popup_region() const;
	void reposition_widget();

	void closeEvent(QCloseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

protected:
	void showEvent(QShowEvent *) override;

Q_SIGNALS:
	void closed();

private:
	QPoint point_;
	PopupPosition pos_;
	bool mouse_pressed_;

};

} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_POPUP_HPP
