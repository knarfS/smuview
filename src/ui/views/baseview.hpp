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

#include <memory>
#include <string>

#include <QMainWindow>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QUuid>
#include <QWidget>

using std::shared_ptr;
using std::string;

namespace sv {

class Session;

namespace devices {
class BaseDevice;
}

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

	QUuid uuid() const;
	string id() const;
	virtual QString title() const = 0;

	virtual void save_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device = nullptr) const;
	virtual void restore_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device = nullptr);

	/** Return a size hint for restoring the correct view size from QSettings. */
	QSize sizeHint() const override;

protected:
	Session &session_;
	QWidget *central_widget_;
	QUuid uuid_;
	string id_;
	/** The size for sizeHint(). */
	QSize size_;

Q_SIGNALS:
	void title_changed();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_BASEVIEW_HPP
