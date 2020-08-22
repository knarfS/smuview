/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_GENERICCONTROLVIEW_HPP
#define UI_VIEWS_GENERICCONTROLVIEW_HPP

#include <memory>

#include <QSettings>
#include <QString>
#include <QUuid>
#include <QWidget>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace devices {
class Configurable;
}

namespace ui {
namespace views {

class GenericControlView : public BaseView
{
	Q_OBJECT

public:
	GenericControlView(Session& session,
		shared_ptr<sv::devices::Configurable> configurable,
		QUuid uuid = QUuid(),
		QWidget* parent = nullptr);

	QString title() const override;

	void save_settings(QSettings &settings) const override;
	void restore_settings(QSettings &settings) override;
	static GenericControlView *init_from_settings(
		Session &session, QSettings &settings, QUuid uuid);

private:
	shared_ptr<sv::devices::Configurable> configurable_;

	void setup_ui();
	void connect_signals();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_GENERICCONTROLVIEW_HPP
