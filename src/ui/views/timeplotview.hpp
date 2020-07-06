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

#ifndef UI_VIEWS_TIMEPLOTVIEW_HPP
#define UI_VIEWS_TIMEPLOTVIEW_HPP

#include <memory>
#include <string>

#include <QSettings>
#include <QUuid>

#include "src/ui/views/baseplotview.hpp"

using std::shared_ptr;
using std::string;

namespace sv {

class Session;

namespace channels {
class BaseChannel;
}
namespace data {
class AnalogTimeSignal;
}

namespace ui {
namespace views {

class TimePlotView : public BasePlotView
{
	Q_OBJECT

public:
	TimePlotView(Session &session, QUuid uuid = QUuid(),
		QWidget *parent = nullptr);

	QString title() const override;

	void save_settings(QSettings &settings) const override;
	void restore_settings(QSettings &settings) override;

	/**
	 * Add a new channel to the time plot.
	 */
	void set_channel(shared_ptr<channels::BaseChannel> channel);
	/**
	 * Add a new signal to the time plot and return the curve id.
	 */
	string add_signal(shared_ptr<sv::data::AnalogTimeSignal> signal);

private:
	shared_ptr<channels::BaseChannel> channel_;

protected Q_SLOTS:
	void on_action_add_signal_triggered() override;

private Q_SLOTS:
	void on_signal_changed();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_TIMEPLOTVIEW_HPP
