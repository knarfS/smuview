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

#ifndef UI_WIDGETS_SCRIPTEDITOR_SMUSCRIPTEDITOR_HPP
#define UI_WIDGETS_SCRIPTEDITOR_SMUSCRIPTEDITOR_HPP

#include <QPlainTextEdit>
#include <QObject>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

namespace sv {
namespace ui {
namespace widgets {
namespace scripteditor {

class PythonSyntaxHighlighter;

class SmuScriptEditor : public QPlainTextEdit
{
	Q_OBJECT

public:
	SmuScriptEditor(QWidget *parent = 0);

	void line_number_area_paint_event(QPaintEvent *event);
	int line_number_area_width();

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	QFont font_;
	QWidget *line_number_area_;
    PythonSyntaxHighlighter *highlighter_;

private Q_SLOTS:
	void update_line_number_area_width(int new_block_count);
	void update_line_number_area(const QRect &, int);
	void highlight_current_line();

};

class LineNumberArea : public QWidget
{

public:
	LineNumberArea(SmuScriptEditor *editor) :
		QWidget(editor),
		editor_(editor)
	{
	}

	QSize sizeHint() const override
	{
		return QSize(editor_->line_number_area_width(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) override
	{
		editor_->line_number_area_paint_event(event);
	}

private:
	SmuScriptEditor *editor_;

};

} // namespace scripteditor
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_SCRIPTEDITOR_SMUSCRIPTEDITOR_HPP
