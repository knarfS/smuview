/*
 * This file is part of the SmuView project.
 *
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

#include <string>

#include <QUuid>

#include "baseview.hpp"
#include "src/session.hpp"

using std::string;

namespace sv {
namespace ui {
namespace views {

BaseView::BaseView(Session &session, QUuid uuid, QWidget *parent) :
	QMainWindow(parent),
	session_(session)
{
	// Every view gets its own unique id
	uuid_ = uuid.isNull() ? QUuid::createUuid() : uuid;

	// Remove Qt::Window flag
	this->setWindowFlags(Qt::Widget);

	// Use a QMainWindow (in the dock widget) to allow for a tool bar
	central_widget_ = new QWidget();
	this->setCentralWidget(central_widget_);
}


Session &BaseView::session()
{
	return session_;
}

const Session &BaseView::session() const
{
	return session_;
}

string BaseView::id() const
{
	return id_;
}

} // namespace views
} // namespace ui
} // namespace sv
