/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_VALUEPANELVIEW_HPP
#define UI_VIEWS_VALUEPANELVIEW_HPP

#include <memory>
#include <set>

#include <QAction>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QToolBar>
#include <QUuid>

#include "src/data/datautil.hpp"
#include "src/ui/views/baseview.hpp"

using std::set;
using std::shared_ptr;

namespace sv {

class Session;

namespace channels {
class BaseChannel;
}
namespace data {
class AnalogTimeSignal;
}
namespace devices {
class BaseDevice;
}

namespace ui {

namespace widgets {
class ValueDisplay;
}

namespace views {

class ValuePanelView : public BaseView
{
	Q_OBJECT

public:
	explicit ValuePanelView(Session& session, QUuid uuid = QUuid(),
		QWidget* parent = nullptr);

	~ValuePanelView();

	QString title() const override;
	void set_channel(shared_ptr<channels::BaseChannel> channel);
	void set_signal(shared_ptr<sv::data::AnalogTimeSignal> signal);

	void save_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device = nullptr) const override;
	void restore_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device = nullptr) override;

private:
	shared_ptr<channels::BaseChannel> channel_;
	shared_ptr<sv::data::AnalogTimeSignal> signal_;

	QTimer *timer_;

	// Min/max/actual values are stored here, so they can be reseted
	double value_min_;
	double value_max_;

	QAction *const action_reset_display_;
	QToolBar *toolbar_;
	widgets::ValueDisplay *value_display_;
	widgets::ValueDisplay *value_min_display_;
	widgets::ValueDisplay *value_max_display_;

	void setup_ui();
	void setup_toolbar();
	void init_displays();
	void connect_signals_channel();
	void disconnect_signals_channel();
	void connect_signals_signal();
	void disconnect_signals_signal();
	void reset_display();
	void init_timer();
	void stop_timer();

private Q_SLOTS:
	void on_update();
	void on_signal_changed();
	void on_action_reset_display_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_VALUEPANELVIEW_HPP
