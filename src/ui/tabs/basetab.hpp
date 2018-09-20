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

#ifndef UI_TABS_BASETAB_HPP
#define UI_TABS_BASETAB_HPP

#include <QDockWidget>
#include <QMainWindow>
#include <QSettings>
#include <QWidget>

#include "src/ui/views/baseview.hpp"

using std::map;

namespace sv {

class Session;

namespace ui {
namespace tabs {

enum class TabType {
	SourceTab,
	SinkTab,
	MeasurementTab,
	ViewsTab
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
	map<QDockWidget *, views::BaseView *> view_docks_;

protected:
	void add_view(views::BaseView *view, Qt::DockWidgetArea area);

	Session &session_;
	QMainWindow *parent_;

};

} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_BASETAB_HPP
