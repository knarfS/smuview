/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef WIDGETS_SINGLEVALUEPANEL_HPP
#define WIDGETS_SINGLEVALUEPANEL_HPP

#include <memory>

#include <QPushButton>
#include <QTimer>
#include <QWidget>

#include "lcddisplay.hpp"

using std::shared_ptr;

namespace sv {

namespace data {
class SignalBase;
}

namespace widgets {

class SingleValuePanel : public QWidget
{
    Q_OBJECT

public:
	SingleValuePanel(shared_ptr<data::SignalBase> value_signal,
		QWidget *parent);
	~SingleValuePanel();

private:
	shared_ptr<data::SignalBase> value_signal_;

	QTimer *timer_;
	qint64 start_time_;
	qint64 last_time_;

	widgets::LcdDisplay *valueDisplay;
	QPushButton *resetButton;

	void setup_ui();
	void reset_display();
	void init_timer();
	void stop_timer();

public Q_SLOTS:
	void on_reset();
	void on_update();
};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_SINGLEVALUEPANEL_HPP

