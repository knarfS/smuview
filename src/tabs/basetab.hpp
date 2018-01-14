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

#ifndef TABS_BASETAB_HPP
#define TABS_BASETAB_HPP

#include <memory>

#include <QDockWidget>
#include <QMainWindow>
#include <QSettings>
#include <QWidget>

#include "src/views/baseview.hpp"

using std::map;
using std::shared_ptr;

namespace sv {

class Session;

namespace tabs {

enum TabType {
	TabTypeSource,
	TabTypeSink,
	TabTypeMeasurement,
	TabTypeViews
};

class BaseTab : public QWidget
{
	Q_OBJECT

public:
	explicit BaseTab(Session &session, QMainWindow *parent = nullptr);

	Session &session();
	const Session &session() const;

	virtual void save_settings(QSettings &settings) const;
	virtual void restore_settings(QSettings &settings);

private:
	map<QDockWidget*, shared_ptr<views::BaseView>> view_docks_;

protected:
	shared_ptr<views::BaseView> add_view(
		shared_ptr<views::BaseView> view, Qt::DockWidgetArea area);

	Session &session_;
	QMainWindow *parent_;

};

} // namespace tabs
} // namespace sv

#endif // TABS_BASETAB_HPP
