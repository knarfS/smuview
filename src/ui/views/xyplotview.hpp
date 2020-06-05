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

#ifndef UI_VIEWS_XYPLOTVIEW_HPP
#define UI_VIEWS_XYPLOTVIEW_HPP

#include <memory>

#include <QSettings>
#include <QUuid>

#include "src/ui/views/baseplotview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace data {
class AnalogTimeSignal;
}

namespace ui {
namespace views {

class XYPlotView : public BasePlotView
{
	Q_OBJECT

public:
	XYPlotView(Session &session, QUuid uuid = QUuid(),
		QWidget *parent = nullptr);

	QString title() const override;

	void save_settings(QSettings &settings) const override;
	void restore_settings(QSettings &settings) override;

	/**
	 * Add a new signal to the xy plot. The new signal will be time correlated
	 * with the already set x signal.
	 */
	void add_signal(shared_ptr<sv::data::AnalogTimeSignal> y_signal);
	/**
	 * Add a new x/y curve to the xy plot.
	 */
	void add_signals(shared_ptr<sv::data::AnalogTimeSignal> x_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_signal);

protected Q_SLOTS:
	void on_action_add_signal_triggered() override;

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_XYPLOTVIEW_HPP
