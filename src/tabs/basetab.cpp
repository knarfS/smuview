/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
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

#include <QMainWindow>
#include <QSizePolicy>

#include "basetab.hpp"
#include "src/session.hpp"

using std::shared_ptr;

namespace sv {
namespace tabs {

BaseTab::BaseTab(Session &session, QMainWindow *parent) :
		QWidget(parent),
	session_(session),
	parent_(parent)
{
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

shared_ptr<views::BaseView> BaseTab::add_view(
	shared_ptr<views::BaseView> view, Qt::DockWidgetArea area)
{
	if (!view)
		return nullptr;

	//GlobalSettings settings;

	// Dock widget must be here, because the layout must be set to the central
	// widget of the view main window before dock->setWidget() is called.
	QDockWidget* dock = new QDockWidget(view->title());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	dock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	dock->setContextMenuPolicy(Qt::PreventContextMenu);
	dock->setFeatures(QDockWidget::DockWidgetMovable |
		QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
	dock->setWidget(view.get());
	parent_->addDockWidget(area, dock);

	// TODO: When removed, the view widget in the dock isn't shown anymore!
	view_docks_[dock] = view;

	return view;
}

} // namespace tabs
} // namespace sv
