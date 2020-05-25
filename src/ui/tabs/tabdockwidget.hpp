/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_TABS_TABDOCKWIDGET_HPP
#define UI_TABS_TABDOCKWIDGET_HPP

#include <string>

#include <QCloseEvent>
#include <QDockWidget>
#include <QString>

using std::string;

namespace sv {
namespace ui {
namespace tabs {

class TabDockWidget : public QDockWidget
{
	Q_OBJECT

public:
	TabDockWidget(const QString &title, const string &view_id,
		QWidget *parent = nullptr);

private:
	void closeEvent(QCloseEvent *event) override;

	string view_id_;

Q_SIGNALS:
	void closed(const std::string &view_id);

};

} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_TABDOCKWIDGET_HPP
