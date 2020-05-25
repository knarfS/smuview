/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <QCloseEvent>
#include <QDebug>
#include <QDockWidget>
#include <QString>

#include "tabdockwidget.hpp"

namespace sv {
namespace ui {
namespace tabs {

TabDockWidget::TabDockWidget(const QString &title, const string &view_id,
		QWidget *parent) :
	QDockWidget(title, parent),
	view_id_(view_id)
{
}

void TabDockWidget::closeEvent(QCloseEvent *event)
{
	Q_EMIT closed(view_id_);
	event->accept();
}

} // namespace tabs
} // namespace ui
} // namespace sv
