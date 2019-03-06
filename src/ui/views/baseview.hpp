/*
 * This file is part of the SmuView project.
 *
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

#ifndef UI_VIEWS_BASEVIEW_HPP
#define UI_VIEWS_BASEVIEW_HPP

#include <QMainWindow>
#include <QSettings>
#include <QWidget>

namespace sv {

class Session;

namespace ui {
namespace views {

enum class ViewType {
	DataView,
	DemoControlView,
	DeviceTreeView,
	MeasurementControlView,
	PlotView,
	PowerPanelView,
	SourceSinkControlView,
	ValuePanelView
};

class BaseView : public QMainWindow
{
	Q_OBJECT

public:
	explicit BaseView(Session &session, QWidget *parent = nullptr);

	Session &session();
	const Session &session() const;

	virtual QString title() const = 0;

	virtual void save_settings(QSettings &settings) const;
	virtual void restore_settings(QSettings &settings);

protected:
	Session &session_;
	QWidget *central_widget_;

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_BASEVIEW_HPP
