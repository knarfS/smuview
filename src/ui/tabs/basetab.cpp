/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
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

#include <QDebug>
#include <QMainWindow>
#include <QSizePolicy>

#include "basetab.hpp"
#include "src/session.hpp"

using std::shared_ptr;

namespace sv {
namespace ui {
namespace tabs {

BaseTab::BaseTab(Session &session, QMainWindow *parent) :
	QWidget(parent),
	session_(session),
	parent_(parent)
{
	// Hide the central widget of the tab, so the  views (dock widgets) can use
	// all of the available space.
	this->hide();
}

Session& BaseTab::session()
{
	return session_;
}

const Session& BaseTab::session() const
{
	return session_;
}

void BaseTab::save_settings(QSettings &settings) const
{
	(void)settings;
}

void BaseTab::restore_settings(QSettings &settings)
{
	(void)settings;
}

views::BaseView *BaseTab::get_view_from_view_id(string id)
{
	return view_id_map_[id];
}

void BaseTab::add_view(views::BaseView *view, Qt::DockWidgetArea area)
{
	if (!view)
		return;

	//GlobalSettings settings;

	// Dock widget must be here, because the layout must be set to the central
	// widget of the view main window before dock->setWidget() is called.
	QDockWidget *dock = new QDockWidget(view->title());
	dock->setAttribute(Qt::WA_DeleteOnClose);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	dock->setContextMenuPolicy(Qt::PreventContextMenu);
	dock->setFeatures(QDockWidget::DockWidgetMovable |
		QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
	dock->setWidget(view);
	parent_->addDockWidget(area, dock);

	// This fixes a qt bug. See: https://bugreports.qt.io/browse/QTBUG-65592
	// resizeDocks() was introduced in Qt 5.6.
	parent_->resizeDocks({dock}, {40}, Qt::Horizontal);

	view_docks_map_[dock] = view;
	view_id_map_[view->id()] = view;
}

} // namespace tabs
} // namespace ui
} // namespace sv
