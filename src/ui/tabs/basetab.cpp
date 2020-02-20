/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QMainWindow>
#include <QSizePolicy>

#include "basetab.hpp"
#include "src/session.hpp"

namespace sv {
namespace ui {
namespace tabs {

BaseTab::BaseTab(Session &session, QWidget *parent) :
	QMainWindow(parent),
	session_(session)
{
	// Remove Qt::Window flag
	this->setWindowFlags(Qt::Widget);
	this->setDockNestingEnabled(true);
	this->setCentralWidget(new QWidget());

	// Hide the central widget of the tab, so the  views (dock widgets) can use
	// all of the available space.
	this->centralWidget()->hide();
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

QDockWidget *BaseTab::create_dock_widget(views::BaseView *view,
	QDockWidget::DockWidgetFeatures features)
{
	// The dock widget must be created here, because the layout must be set to
	// the central widget of the view main window before dock->setWidget() is
	// called.
	// Otherwise the application will flicker at startup....
	QDockWidget *dock = new QDockWidget(view->title());
	dock->setAttribute(Qt::WA_DeleteOnClose);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	dock->setContextMenuPolicy(Qt::PreventContextMenu);
	dock->setFeatures(features);
	dock->setWidget(view);

	view_docks_map_[view] = dock;
	view_id_map_[view->id()] = view;

	return dock;
}

void BaseTab::add_view(views::BaseView *view, Qt::DockWidgetArea area,
	int features)
{
	if (!view)
		return;

	//GlobalSettings settings;

	QDockWidget *dock = create_dock_widget(
		view, (QDockWidget::DockWidgetFeatures)features);
	this->addDockWidget(area, dock);

	// This fixes a qt bug. See: https://bugreports.qt.io/browse/QTBUG-65592
	// resizeDocks() was introduced in Qt 5.6.
	this->resizeDocks({dock}, {40}, Qt::Horizontal);
}

void BaseTab::add_view_ontop(views::BaseView *view,
	views::BaseView *existing_view, int features)
{
	if (!view)
		return;

	//GlobalSettings settings;

	QDockWidget *dock = create_dock_widget(
		view, (QDockWidget::DockWidgetFeatures)features);
	this->tabifyDockWidget(view_docks_map_[existing_view], dock);

	// This fixes a qt bug. See: https://bugreports.qt.io/browse/QTBUG-65592
	// resizeDocks() was introduced in Qt 5.6.
	this->resizeDocks({dock}, {40}, Qt::Horizontal);
}

} // namespace tabs
} // namespace ui
} // namespace sv
