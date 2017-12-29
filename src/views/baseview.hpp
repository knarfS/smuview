/*
 * This file is part of the SmuView project.
 *
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

#ifndef VIEWS_BASEVIEW_HPP
#define VIEWS_BASEVIEW_HPP

#include <QMainWindow>
#include <QSettings>
#include <QWidget>

namespace sv {

class Session;

namespace views {

enum ViewType {
	ViewTypeMeasurementControl,
	ViewTypeSinkControl,
	ViewTypeSourceControl,
	ViewTypeValuePanel,
	ViewTypePowerPanel,
	ViewTypePlot
};

class BaseView : public QMainWindow
{
	Q_OBJECT

public:
	explicit BaseView(const Session &session, QWidget *parent = nullptr);

	const Session& session() const;

	virtual void save_settings(QSettings &settings) const;
	virtual void restore_settings(QSettings &settings);

protected:
	const Session &session_;

	QWidget *centralWidget;

private:
	static const int MaxViewAutoUpdateRate;

public Q_SLOTS:

private Q_SLOTS:

};

} // namespace views
} // namespace sv

#endif // VIEWS_BASEVIEW_HPP
