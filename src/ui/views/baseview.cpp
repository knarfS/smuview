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

#include <memory>
#include <string>

#include <QSettings>
#include <QSize>
#include <QString>
#include <QUuid>
#include <QVariant>

#include "baseview.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"

using std::shared_ptr;
using std::string;

namespace sv {
namespace ui {
namespace views {

BaseView::BaseView(Session &session, QUuid uuid, QWidget *parent) :
	QMainWindow(parent),
	session_(session),
	size_(QSize(-1, -1))
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

QUuid BaseView::uuid() const
{
	return uuid_;
}

string BaseView::id() const
{
	return id_;
}

void BaseView::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	(void)origin_device;

	settings.setValue("uuid", QVariant(uuid()));
	settings.setValue("id", QVariant(QString::fromStdString(id())));
	// NOTE: The size must be saved together with the geometry (saveGeometry())
	//       of all dock widgets, see DeviceTab::save_settings().
	settings.setValue("size", size());
}

void BaseView::restore_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	(void)origin_device;

	// NOTE: The size must be restored together with the geometry
	//       (restoreGeometry()) of all dock widgets, see
	//       DeviceTab::restore_settings().
	size_ = settings.value("size").toSize();
}

QSize BaseView::sizeHint() const
{
	if (size_.width() >= 0 && size_.height() >= 0)
		return size_;
	return QMainWindow::sizeHint();
}

} // namespace views
} // namespace ui
} // namespace sv
