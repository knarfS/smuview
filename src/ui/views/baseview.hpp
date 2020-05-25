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

#ifndef UI_VIEWS_BASEVIEW_HPP
#define UI_VIEWS_BASEVIEW_HPP

#include <string>

#include <QMainWindow>
#include <QSettings>
#include <QUuid>
#include <QWidget>

using std::string;

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
	explicit BaseView(Session &session, QUuid uuid = QUuid(),
		QWidget *parent = nullptr);

	Session &session();
	const Session &session() const;

	string id() const;
	virtual QString title() const = 0;

	// TODO: scope
	virtual void save_settings(QSettings &settings) const = 0;
	virtual void restore_settings(QSettings &settings) = 0;

protected:
	Session &session_;
	QWidget *central_widget_;
	QUuid uuid_;
	string id_;

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_BASEVIEW_HPP
