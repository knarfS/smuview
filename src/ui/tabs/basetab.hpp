/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_TABS_BASETAB_HPP
#define UI_TABS_BASETAB_HPP

#include <string>

#include <QDockWidget>
#include <QMainWindow>
#include <QSettings>
#include <QWidget>

#include "src/ui/views/baseview.hpp"

using std::map;
using std::string;

namespace sv {

class Session;

namespace ui {
namespace tabs {

enum class TabType {
	MeasurementTab,
	SourceSinkTab,
	UserTab,
	WelcomeTab
};

/**
 * Use a QMainWindow (as tab widget) to allow for a tool bar.
 */
class BaseTab : public QMainWindow
{
	Q_OBJECT

public:
	explicit BaseTab(Session &session, QWidget *parent = nullptr);

	Session &session();
	const Session &session() const;

	virtual void save_settings(QSettings &settings) const;
	virtual void restore_settings(QSettings &settings);
	views::BaseView *get_view_from_view_id(string id);
	virtual bool request_close() = 0;

private:
	map<QDockWidget *, views::BaseView *> view_docks_map_;
	map<string, views::BaseView *> view_id_map_;

protected:
	Session &session_;

public Q_SLOTS:
	void add_view(views::BaseView *view, Qt::DockWidgetArea area);

};

} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_BASETAB_HPP
