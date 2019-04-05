/*
 * This file is part of the SmuView project.
 * This file is based on the Qt "Code Editor Example"
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QLatin1Char>
#include <QList>
#include <QPainter>
#include <QPlainTextEdit>
#include <QRect>
#include <QResizeEvent>
#include <QString>
#include <QTextBlock>
#include <QTextEdit>
#include <QTextFormat>
#include <QWidget>

#include "smuscripteditor.hpp"
#include "src/ui/widgets/scripteditor/pythonsyntaxhighlighter.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace scripteditor {

SmuScriptEditor::SmuScriptEditor(QWidget *parent) : QPlainTextEdit(parent)
{
	font_ = QFont("Monospace");
	font_.setStyleHint(QFont::TypeWriter);
	font_.setFixedPitch(true);
	this->setFont(font_);

	line_number_area_ = new LineNumberArea(this);
	highlighter_ = new PythonSyntaxHighlighter(this->document());

	connect(this, SIGNAL(blockCountChanged(int)),
		this, SLOT(update_line_number_area_width(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)),
		this, SLOT(update_line_number_area(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()),
		this, SLOT(highlight_current_line()));

	update_line_number_area_width(0);
	highlight_current_line();
}

int SmuScriptEditor::line_number_area_width()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) {
		max /= 10;
		++digits;
	}
	int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

	return space;
}

void SmuScriptEditor::update_line_number_area_width(int new_block_count)
{
	(void)new_block_count;
	setViewportMargins(line_number_area_width(), 0, 0, 0);
}

void SmuScriptEditor::update_line_number_area(const QRect &rect, int dy)
{
	if (dy) {
		line_number_area_->scroll(0, dy);
	}
	else {
		line_number_area_->update(
			0, rect.y(), line_number_area_->width(), rect.height());
	}

	if (rect.contains(viewport()->rect()))
		update_line_number_area_width(0);
}

void SmuScriptEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	line_number_area_->setGeometry(
		QRect(cr.left(), cr.top(), line_number_area_width(), cr.height()));
}

void SmuScriptEditor::highlight_current_line()
{
	QList<QTextEdit::ExtraSelection> extra_selections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::yellow).lighter(160);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extra_selections.append(selection);
	}

	setExtraSelections(extra_selections);
}

void SmuScriptEditor::line_number_area_paint_event(QPaintEvent *event)
{
	// Get standard background color
	QColor bk_color = QWidget::palette().color(QWidget::backgroundRole());
	QPainter painter(line_number_area_);
	painter.fillRect(event->rect(), bk_color);

	QTextBlock block = firstVisibleBlock();
	int block_number = block.blockNumber();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();

	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(block_number + 1);
			painter.setPen(Qt::darkGray);
			painter.drawText(0, top, line_number_area_->width(),
				fontMetrics().height(), Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++block_number;
	}
}

} // namespace scripteditor
} // namespace widgets
} // namespace ui
} // namespace sv
