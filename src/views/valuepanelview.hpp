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

#ifndef VIEWS_VALUEPANELVIEW_HPP
#define VIEWS_VALUEPANELVIEW_HPP

#include <memory>

#include <QPushButton>
#include <QTimer>

#include "src/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace data {
class BaseSignal;
}

namespace widgets {
class LcdDisplay;
}

namespace views {

class ValuePanelView : public BaseView
{
	Q_OBJECT

public:
	ValuePanelView(Session& session,
		shared_ptr<data::BaseSignal> value_signal,
		QWidget* parent);
	~ValuePanelView();

private:
	uint digits_;
	QString unit_;
	shared_ptr<data::BaseSignal> value_signal_;

	QTimer *timer_;

	widgets::LcdDisplay *valueDisplay;
	QPushButton *resetButton;

	void setup_ui();
	void connect_signals();
	void reset_display();
	void init_timer();
	void stop_timer();

protected:

public Q_SLOTS:

private Q_SLOTS:
	void on_reset();
	void on_update();

};

} // namespace views
} // namespace sv

#endif // VIEWS_VALUEPANELVIEW_HPP

